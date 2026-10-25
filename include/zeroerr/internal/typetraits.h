#pragma once
#include "zeroerr/internal/config.h"



#include "zeroerr/string.h"
#include <tuple>  // this should be removed
#include <type_traits>

#include <memory>
#include <cstddef>
#include <utility>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH


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


class StringBuffer;
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
struct gen_seq<0, Is...> : seq<Is...> { using type = seq<Is...>; };


// Some utility structs to check template specialization
template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};


// Check if a type is stream writable, i.e., std::cout << foo;
// Usage: is_streamable<std::ostream, int>::value
template <typename S, typename T>
using has_stream_operator = void_t<decltype(std::declval<S&>() << std::declval<const T&>())>;

template <typename S, typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename S, typename T>
struct is_streamable<S, T, has_stream_operator<S, T>> : std::true_type {};


// Check if a type is a container type
// Usage: is_container<std::vector<int>>::value
template <typename T>
using has_begin_end =
    void_t<decltype(std::declval<const T&>().begin()),
           decltype(std::declval<const T&>().end()),
           decltype(*std::declval<decltype(std::declval<const T&>().begin())&>()),
           decltype(++std::declval<decltype(std::declval<const T&>().begin())&>()),
           decltype(std::declval<const T&>().begin() != std::declval<const T&>().end()),
           decltype(std::declval<const T&>().begin() == std::declval<const T&>().end())>;

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


template <typename T, typename = void>
struct is_string_like : std::false_type {};

template <typename T>
struct is_string_like<T, void_t<typename T::traits_type::char_type,
                                decltype(std::declval<const T&>().data()),
                                decltype(std::declval<const T&>().size())>>
    : std::integral_constant<bool,
          std::is_same<typename T::traits_type::char_type, char>::value &&
          std::is_convertible<decltype(std::declval<const T&>().data()), const char*>::value &&
          std::is_convertible<decltype(std::declval<const T&>().size()), std::size_t>::value> {};

template<> struct is_string_like<zeroerr::String> : std::true_type {};

template <class T>
struct is_string : std::integral_constant<bool,
    is_string_like<T>::value ||
    (std::is_convertible<const T&, const char*>::value && !std::is_same<T, std::nullptr_t>::value)> {};

template <typename T, typename = void>
struct is_complex : std::false_type {};

template <typename T>
struct is_complex<T, void_t<decltype(std::declval<const T&>().real()),
                            decltype(std::declval<const T&>().imag())>>
    : std::integral_constant<bool,
          std::is_arithmetic<decltype(std::declval<const T&>().real())>::value &&
          std::is_arithmetic<decltype(std::declval<const T&>().imag())>::value> {};

template <typename T, typename = void>
struct is_pointer_like : std::false_type {};

template <typename T>
struct is_pointer_like<T, void_t<decltype(std::declval<const T&>().get())>>
    : std::is_convertible<decltype(std::declval<const T&>().get()), const void*> {};

template <typename T, typename = void>
struct is_weak_pointer_like : std::false_type {};

template <typename T>
struct is_weak_pointer_like<T, void_t<decltype(std::declval<const T&>().lock())>>
    : is_pointer_like<decltype(std::declval<const T&>().lock())> {};

// Unqualified, dependent lookup finds get in the value's associated namespace,
// including declarations introduced after this header. Never declare in std.
template <std::size_t I>
void get() = delete;
template <std::size_t I, typename T>
auto tuple_get(T&& value) -> decltype(get<I>(std::forward<T>(value))) {
    return get<I>(std::forward<T>(value));
}

template <typename T, typename Sequence, typename = void>
struct has_tuple_get : std::false_type {};

template <typename T, unsigned... I>
struct has_tuple_get<T, seq<I...>, void_t<decltype(tuple_get<I>(std::declval<const T&>()))...>>
    : std::true_type {};

template <typename T, typename = void>
struct is_tuple_like : std::false_type {};

template <typename T>
struct is_tuple_like<T, void_t<decltype(std::tuple_size<T>::value)>>
    : has_tuple_get<T, typename gen_seq<std::tuple_size<T>::value>::type> {};

// Completeness check that SFINAE-fails for incomplete T.
template <typename T, typename = void>
struct is_complete_type : std::false_type {};
template <typename T>
struct is_complete_type<T, decltype(void(sizeof(T)))> : std::true_type {};

// Non-pointer [0] detection. Must not be instantiated for pointer-to-incomplete:
// GCC treats that subscript as a hard error inside void_t, not SFINAE.
template <typename T, typename = void>
struct is_indexable_non_pointer : std::false_type {};
template <typename T>
struct is_indexable_non_pointer<T, void_t<decltype(std::declval<const T&>()[0])>> : std::true_type {};

// Dispatch on is_pointer first so pointer-to-incomplete never reaches [0].
template <typename T,
          bool = std::is_pointer<
              typename std::remove_cv<typename std::remove_reference<T>::type>::type>::value>
struct is_array_dispatch : is_indexable_non_pointer<T> {};

template <typename T>
struct is_array_dispatch<T, true>
    : is_complete_type<typename std::remove_pointer<
          typename std::remove_cv<typename std::remove_reference<T>::type>::type>::type> {};

// Check if a type can use arr[0] like an array.
template <typename T>
struct is_array : is_array_dispatch<T> {};


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
    void_t<decltype((*std::declval<const T&>().begin()).first),
           decltype((*std::declval<const T&>().begin()).second)>;

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
    using type = zeroerr::String;
};

template <>
struct to_store_type<const char (&)[]> {
    using type = zeroerr::String;
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
            fun(tuple_get<I - 1>(tup));
        else
            visit_impl<I - 1>::visit(tup, idx, std::forward<F>(fun));
    }
};

template <>
struct visit_impl<0> {
    template <typename T, typename F>
    static void visit(T&, size_t, F&&) {}
};

template <typename F, typename T>
void visit_at(T& tup, size_t idx, F&& fun) {
    visit_impl<std::tuple_size<typename std::remove_cv<T>::type>::value>::visit(
        tup, idx, std::forward<F>(fun));
}

template <size_t I>
struct visit2_impl {
    template <typename T1, typename T2, typename F>
    static void visit(T1& tup1, T2& tup2, size_t idx, F&& fun) {
        if (idx == I - 1)
            fun(tuple_get<I - 1>(tup1), tuple_get<I - 1>(tup2));
        else
            visit2_impl<I - 1>::visit(tup1, tup2, idx, std::forward<F>(fun));
    }
};

template <>
struct visit2_impl<0> {
    template <typename T1, typename T2, typename F>
    static void visit(T1&, T2&, size_t, F&&) {}
};

template <typename F, typename T1, typename T2>
void visit2_at(T1& tup1, T2& tup2, size_t idx, F&& fun) {
    visit2_impl<std::tuple_size<typename std::remove_cv<T1>::type>::value>::visit(
        tup1, tup2, idx, std::forward<F>(fun));
}

#define ZEROERR_ENABLE_IF(x) \
    template <typename T>    \
    typename std::enable_if<x, void>::type
#define ZEROERR_IS_INT        std::is_integral<T>::value
#define ZEROERR_IS_FLOAT      std::is_floating_point<T>::value
#define ZEROERR_IS_ENUM       std::is_enum<T>::value
#define ZEROERR_IS_CONTAINER  detail::is_container<T>::value
#define ZEROERR_IS_STRING     detail::is_string<T>::value
#define ZEROERR_IS_POINTER    (std::is_pointer<T>::value || std::is_same<T, std::nullptr_t>::value)
#define ZEROERR_IS_CHAR       std::is_same<T, char>::value
#define ZEROERR_IS_WCHAR      std::is_same<T, wchar_t>::value
#define ZEROERR_IS_CLASS      std::is_class<T>::value
#define ZEROERR_IS_STREAMABLE detail::is_streamable<zeroerr::StringBuffer, T>::value
#define ZEROERR_IS_ARRAY      detail::is_array<T>::value
#define ZEROERR_IS_COMPLEX    detail::is_complex<T>::value
#define ZEROERR_IS_BOOL       std::is_same<T, bool>::value
#define ZEROERR_IS_AUTOPTR detail::is_pointer_like<T>::value
#define ZEROERR_IS_MAP detail::ele_type_is_pair<T>::value
#define ZEROERR_IS_POD std::is_standard_layout<T>::value
#define ZEROERR_IS_EXT detail::has_extension<T>::value

}  // namespace detail

}  // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP