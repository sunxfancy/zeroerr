#pragma once
#include "zeroerr/internal/config.h"


#include <type_traits>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>  // this should be removed

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


namespace detail {

// C++11 void_t
template <typename... Ts>
using void_t = void;


// Some utility structs to check template specialization
template <typename Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};


// Check if a type is stream writable, i.e., std::cout << foo;
// Usage: is_streamable<std::ostream, int>::value
template <typename S, typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename S, typename T>
struct is_streamable<S, T, void_t<decltype(std::declval<S&>() << std::declval<T>())>>
    : std::true_type {};


// Check if a type is a container type
// Usage: is_container<std::vector<int>>::value
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T,
                    void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
    : std::true_type {};

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


// Check if a type has the element type as std::pair
template <typename T, typename = void>
struct ele_type_is_pair : std::false_type {};

template <typename T>
struct ele_type_is_pair<
    T, void_t<typename T::value_type, decltype(std::declval<typename T::value_type>().first),
              decltype(std::declval<typename T::value_type>().second)>> : std::true_type {};




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

} // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP