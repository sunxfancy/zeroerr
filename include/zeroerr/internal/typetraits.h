#pragma once
#include "zeroerr/internal/config.h"

#include <ostream>
#include <sstream>
#include <string>
#include <tuple>  // this should be removed
#include <type_traits>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

// those predefines can help to avoid include too many headers
namespace std {
template <typename T>
class complex;

template <class T, class Deleter>
class unique_ptr;

template <class T>
class shared_ptr;

template <class T>
class weak_ptr;

}  // namespace std


namespace zeroerr {


/**
 * @brief rank is a helper class for Printer to define the priority of overloaded functions.
 * @tparam N the priority of the rule. 0 is the lowest priority. The maximum priority is max_rank.
 *
 * You can define a rule by adding it as a function parameter with rank<N> where N is the priority.
 * For example:
 * template<typename T>
 * void Foo(T v, rank<0>); // lowest priority
 * void Foo(int v, rank<1>); // higher priority
 *
 * Even though in the first rule, type T can be an int, the second function will still be called due
 * to the priority.
 */
template <unsigned N>
struct rank : rank<N - 1> {};
template <>
struct rank<0> {};


namespace detail {

// C++11 void_t
template <typename... Ts>
using void_t = void;

// Type dependent "true"/"false".
// Useful for SFINAE and static_asserts where we need a type dependent
// expression that happens to be constant.
template <typename T>
struct always_false {
    static std::false_type value;
};

template <typename T>
struct always_true {
    static std::true_type value;
};


// Generate sequence of integers from 0 to N-1
// Usage: detail::gen_seq<N>  then use <size_t... I> to match it
template <unsigned...>
struct seq {};

template <unsigned N, unsigned... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

template <unsigned... Is>
struct gen_seq<0, Is...> : seq<Is...> {};


// Some utility structs to check template specialization
template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};


// Check if a type is stream writable, i.e., std::cout << foo;
// Usage: is_streamable<std::ostream, int>::value
template <typename S, typename T>
using has_stream_operator = void_t<decltype(std::declval<S&>() << std::declval<T>())>;

template <typename S, typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename S, typename T>
struct is_streamable<S, T, has_stream_operator<S, T>> : std::true_type {};


// Check if a type is a container type
// Usage: is_container<std::vector<int>>::value
template <typename T>
using has_begin_end =
    void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>;

template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, has_begin_end<T>> : std::true_type {};


template <typename T>
using has_begin_end_find_insert =
    void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()),
           decltype(std::declval<T>().find(std::declval<typename T::key_type>())),
           decltype(std::declval<T>().insert(std::declval<typename T::value_type>()))>;

template <typename T, typename = void>
struct is_associative_container : std::false_type {};

template <typename T>
struct is_associative_container<T, has_begin_end_find_insert<T>> : std::true_type {};


#if ZEROERR_CXX_STANDARD >= 17
#define ZEROERR_STRING_VIEW std::is_same<T, std::string_view>::value
#else
#define ZEROERR_STRING_VIEW 0
#endif

// Check if a type is a string type
template <class T>
struct is_string
    : std::integral_constant<bool, std::is_same<T, std::string>::value ||
                                       std::is_same<T, const char*>::value || ZEROERR_STRING_VIEW> {
};


// Check if a type can use arr[0] like an array
template <typename T, typename = void>
struct is_array : std::false_type {};

template <typename T>
struct is_array<T, void_t<decltype(std::declval<T>()[0])>> : std::true_type {};


template <typename T, typename = void>
struct is_modifiable : std::false_type {};

template <typename T>
struct is_modifiable<T, void_t<decltype(
                                // Iterable
                                T().begin(), T().end(), T().size(),
                                // Values are mutable
                                // This rejects associative containers, for example
                                // *T().begin() = std::declval<value_type_t<T>>(),
                                // Can insert and erase elements
                                T().insert(T().end(), std::declval<typename T::value_type>()),
                                T().erase(T().begin()), (void)0)>> : std::true_type {};



// Check if a type has the element type as std::pair
template <typename T>
using has_pair_type =
    void_t<typename T::value_type, decltype(std::declval<typename T::value_type>().first),
           decltype(std::declval<typename T::value_type>().second)>;
template <typename T, typename = void>
struct ele_type_is_pair : std::false_type {};

template <typename T>
struct ele_type_is_pair<T, has_pair_type<T>> : std::true_type {};

template <typename T, typename V = void>
struct to_store_type {
    using type = T;
};

template <>
struct to_store_type<const char*> {
    using type = std::string;
};

template <>
struct to_store_type<const char (&)[]> {
    using type = std::string;
};

template <typename T>
using is_not_array = typename std::enable_if<!std::is_array<T>::value>::type;
template <typename T>
struct to_store_type<T&, is_not_array<T>> {
    using type = T;
};

template <typename T>
struct to_store_type<T&&> {
    using type = T;
};

template <typename T>
using to_store_type_t = typename to_store_type<T>::type;


template <size_t I>
struct visit_impl {
    template <typename T, typename F>
    static void visit(T& tup, size_t idx, F&& fun) {
        if (idx == I - 1)
            fun(std::get<I - 1>(tup));
        else
            visit_impl<I - 1>::visit(tup, idx, std::forward<F>(fun));
    }
};

template <>
struct visit_impl<0> {
    template <typename T, typename F>
    static void visit(T&, size_t, F&&) {}
};

template <typename F, typename... Ts>
void visit_at(const std::tuple<Ts...>& tup, size_t idx, F&& fun) {
    visit_impl<sizeof...(Ts)>::visit(tup, idx, std::forward<F>(fun));
}

template <typename F, typename... Ts>
void visit_at(std::tuple<Ts...>& tup, size_t idx, F&& fun) {
    visit_impl<sizeof...(Ts)>::visit(tup, idx, std::forward<F>(fun));
}


template <size_t I>
struct visit2_impl {
    template <typename T1, typename T2, typename F>
    static void visit(T1 tup1, T2 tup2, size_t idx, F&& fun) {
        if (idx == I - 1)
            fun(std::get<I - 1>(tup1), std::get<I - 1>(tup2));
        else
            visit2_impl<I - 1>::visit(tup1, tup2, idx, std::forward<F>(fun));
    }
};

template <>
struct visit2_impl<0> {
    template <typename T1, typename T2, typename F>
    static void visit(T1&, T2&, size_t, F&&) {}
};

template <typename F, typename... Ts, typename... T2s>
void visit2_at(const std::tuple<Ts...>& tup1, const std::tuple<T2s...>& tup2, size_t idx, F&& fun) {
    visit2_impl<sizeof...(Ts)>::visit(tup1, tup2, idx, std::forward<F>(fun));
}

template <typename F, typename... Ts, typename... T2s>
void visit2_at(std::tuple<Ts...>& tup1, std::tuple<T2s...>& tup2, size_t idx, F&& fun) {
    visit2_impl<sizeof...(Ts)>::visit(tup1, tup2, idx, std::forward<F>(fun));
}

template <typename F, typename... Ts, typename... T2s>
void visit2_at(const std::tuple<Ts...>& tup1, std::tuple<T2s...>& tup2, size_t idx, F&& fun) {
    visit2_impl<sizeof...(Ts)>::visit(tup1, tup2, idx, std::forward<F>(fun));
}

template <typename F, typename... Ts, typename... T2s>
void visit2_at(std::tuple<Ts...>& tup1, const std::tuple<T2s...>& tup2, size_t idx, F&& fun) {
    visit2_impl<sizeof...(Ts)>::visit(tup1, tup2, idx, std::forward<F>(fun));
}

#define ZEROERR_ENABLE_IF(x) \
    template <typename T>    \
    typename std::enable_if<x, void>::type
#define ZEROERR_IS_INT        std::is_integral<T>::value
#define ZEROERR_IS_FLOAT      std::is_floating_point<T>::value
#define ZEROERR_IS_CONTAINER  detail::is_container<T>::value
#define ZEROERR_IS_STRING     detail::is_string<T>::value
#define ZEROERR_IS_POINTER    std::is_pointer<T>::value
#define ZEROERR_IS_CHAR       std::is_same<T, char>::value
#define ZEROERR_IS_WCHAR      std::is_same<T, wchar_t>::value
#define ZEROERR_IS_CLASS      std::is_class<T>::value
#define ZEROERR_IS_STREAMABLE detail::is_streamable<std::ostream, T>::value
#define ZEROERR_IS_ARRAY      detail::is_array<T>::value
#define ZEROERR_IS_COMPLEX    detail::is_specialization<T, std::complex>::value
#define ZEROERR_IS_BOOL       std::is_same<T, bool>::value
#define ZEROERR_IS_AUTOPTR                                   \
    (detail::is_specialization<T, std::unique_ptr>::value || \
     detail::is_specialization<T, std::shared_ptr>::value || \
     detail::is_specialization<T, std::weak_ptr>::value)
#define ZEROERR_IS_MAP detail::ele_type_is_pair<T>::value
#define ZEROERR_IS_POD std::is_standard_layout<T>::value
#define ZEROERR_IS_EXT detail::has_extension<T>::value

}  // namespace detail

}  // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP