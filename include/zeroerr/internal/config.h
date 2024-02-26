#pragma once


// If you just wish to use the color without dynamic
// enable or disable it, you can uncomment the following line
// #define ZEROERR_ALWAYS_COLORFUL
// #define ZEROERR_DISABLE_COLORFUL

// If you wish to use the whole library without thread safety, uncomment the following line
// #define ZEROERR_NO_THREAD_SAFE

// If you wish to disable auto initialization of the system
// #define ZEROERR_DISABLE_AUTO_INIT

// If you didn't wish override operator<< for ostream, we can disable it
// #define ZEROERR_DISABLE_OSTREAM_OVERRIDE

// If you wish to disable AND, OR macro
// #define ZEROERR_DISABLE_COMPLEX_AND_OR


// Detect C++ standard
#if __cplusplus >= 201703L
#define ZEROERR_CXX_STANDARD 17
#elif __cplusplus >= 201402L
#define ZEROERR_CXX_STANDARD 14
#else
#define ZEROERR_CXX_STANDARD 11
#endif


#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define ZEROERR_OS_UNIX
#if defined(__linux__)
#define ZEROERR_OS_LINUX
#endif
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define ZEROERR_OS_WINDOWS
#else
#define ZEROERR_OS_UNKNOWN
#endif


#if defined(NDEBUG) && !defined(ZEROERR_ALWAYS_ASSERT)
// FIXME: we should safely remove the assert in IF statement
// #define ZEROERR_NO_ASSERT
#endif

// This is used for generating a unique name based on the file name and line number
#define ZEROERR_CAT_IMPL(s1, s2) s1##s2
#define ZEROERR_CAT(x, s)        ZEROERR_CAT_IMPL(x, s)

// The following macros are used to check the arguments is empty or not
// from: https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
#define _ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define HAS_COMMA(...)                                                                    _ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define _TRIGGER_PARENTHESIS_(...)                                                        ,

#define ISEMPTY(...)                                                                               \
    _ISEMPTY(                        /* test if there is just one argument, eventually an empty    \
                                        one */                                                     \
             HAS_COMMA(__VA_ARGS__), /* test if _TRIGGER_PARENTHESIS_ together with the argument   \
                                        adds a comma */                                            \
             HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__), /* test if the argument together with a \
                                                              parenthesis adds a comma */          \
             HAS_COMMA(                                                                            \
                 __VA_ARGS__(/*empty*/)), /* test if placing it between _TRIGGER_PARENTHESIS_ and  \
                                             the parenthesis adds a comma */                       \
             HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__(/*empty*/)))

#define PASTE5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4
#define _ISEMPTY(_0, _1, _2, _3)   HAS_COMMA(PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001        ,


// The counter is used to generate a unique name
#ifdef __COUNTER__
#define ZEROERR_NAMEGEN(x) ZEROERR_CAT(x, __COUNTER__)
#else  // __COUNTER__
#define ZEROERR_NAMEGEN(x) ZEROERR_CAT(x, __LINE__)
#endif  // __COUNTER__

#ifdef ZEROERR_OS_LINUX
#define ZEROERR_PERF
#endif

#ifdef ZEROERR_DISABLE_ASSERTS_RETURN_VALUES
#define ZEROERR_FUNC_SCOPE_BEGIN  do
#define ZEROERR_FUNC_SCOPE_END    while (0)
#define ZEROERR_FUNC_SCOPE_RET(v) (void)0
#else
#define ZEROERR_FUNC_SCOPE_BEGIN  [&]
#define ZEROERR_FUNC_SCOPE_END    ()
#define ZEROERR_FUNC_SCOPE_RET(v) return v
#endif

#ifndef ZEROERR_NO_SHORT_LOG_MACRO
#define ZEROERR_USE_SHORT_LOG_MACRO
#endif

#define ZEROERR_EXPAND(x) x

namespace zeroerr {
namespace detail {

// Generate sequence of integers from 0 to N-1
// Usage: detail::gen_seq<N>  then use <size_t... I> to match it
template <unsigned...>
struct seq {};

template <unsigned N, unsigned... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

template <unsigned... Is>
struct gen_seq<0, Is...> : seq<Is...> {};

}  // namespace detail
}  // namespace zeroerr