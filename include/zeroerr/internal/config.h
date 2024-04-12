#pragma once

#define ZEROERR_VERSION_MAJOR 0
#define ZEROERR_VERSION_MINOR 2
#define ZEROERR_VERSION_PATCH 1
#define ZEROERR_VERSION \
    (ZEROERR_VERSION_MAJOR * 10000 + ZEROERR_VERSION_MINOR * 100 + ZEROERR_VERSION_PATCH)

#define ZEROERR_STR(x) #x

#define ZEROERR_VERSION_STR_BUILDER(a, b, c) ZEROERR_STR(a) "." ZEROERR_STR(b) "." ZEROERR_STR(c)
#define ZEROERR_VERSION_STR \
    ZEROERR_VERSION_STR_BUILDER(ZEROERR_VERSION_MAJOR, ZEROERR_VERSION_MINOR, ZEROERR_VERSION_PATCH)

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


// Detect C++ standard with a cross-platform way

#ifdef _MSC_VER
#define ZEROERR_CPLUSPLUS _MSVC_LANG
#else
#define ZEROERR_CPLUSPLUS __cplusplus
#endif

#if ZEROERR_CPLUSPLUS >= 202300L
#define ZEROERR_CXX_STANDARD 23
#elif ZEROERR_CPLUSPLUS >= 202002L
#define ZEROERR_CXX_STANDARD 20
#elif ZEROERR_CPLUSPLUS >= 201703L
#define ZEROERR_CXX_STANDARD 17
#elif ZEROERR_CPLUSPLUS >= 201402L
#define ZEROERR_CXX_STANDARD 14
#elif ZEROERR_CPLUSPLUS >= 201103L
#define ZEROERR_CXX_STANDARD 11
#else
#error "Unsupported C++ standard detected. ZeroErr requires C++11 or later."
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
#define ZEROERR_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define ZEROERR_HAS_COMMA(...) \
    ZEROERR_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define ZEROERR_TRIGGER_PARENTHESIS_(...) ,

#define ZEROERR_ISEMPTY(...)                                                                     \
    _ZEROERR_ISEMPTY(/* test if there is just one argument, eventually an empty                  \
                one */                                                                           \
                     ZEROERR_HAS_COMMA(__VA_ARGS__), /* test if ZEROERR_TRIGGER_PARENTHESIS_     \
                                                together with the argument adds a comma */       \
                     ZEROERR_HAS_COMMA(ZEROERR_TRIGGER_PARENTHESIS_                              \
                                           __VA_ARGS__), /* test if the argument together with   \
                                             a parenthesis adds a comma */                       \
                     ZEROERR_HAS_COMMA(__VA_ARGS__(                                              \
                         /*empty*/)), /* test if placing it between ZEROERR_TRIGGER_PARENTHESIS_ \
                                         and the parenthesis adds a comma */                     \
                     ZEROERR_HAS_COMMA(ZEROERR_TRIGGER_PARENTHESIS_ __VA_ARGS__(/*empty*/)))

#define ZEROERR_PASTE5(_0, _1, _2, _3, _4) _0##_1##_2##_3##_4
#define _ZEROERR_ISEMPTY(_0, _1, _2, _3) \
    ZEROERR_HAS_COMMA(ZEROERR_PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,


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


// =================================================================================================
// == COMPILER Detector ============================================================================
// =================================================================================================

#define ZEROERR_COMPILER(MAJOR, MINOR, PATCH) ((MAJOR) * 10000000 + (MINOR) * 100000 + (PATCH))

// GCC/Clang and GCC/MSVC are mutually exclusive, but Clang/MSVC are not because of clang-cl...
#if defined(_MSC_VER) && defined(_MSC_FULL_VER)
#if _MSC_VER == _MSC_FULL_VER / 10000
#define ZEROERR_MSVC ZEROERR_COMPILER(_MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 10000)
#else  // MSVC
#define ZEROERR_MSVC \
    ZEROERR_COMPILER(_MSC_VER / 100, (_MSC_FULL_VER / 100000) % 100, _MSC_FULL_VER % 100000)
#endif  // MSVC
#endif  // MSVC
#if defined(__clang__) && defined(__clang_minor__) && defined(__clang_patchlevel__)
#define ZEROERR_CLANG ZEROERR_COMPILER(__clang_major__, __clang_minor__, __clang_patchlevel__)
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__) && \
    !defined(__INTEL_COMPILER)
#define ZEROERR_GCC ZEROERR_COMPILER(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#endif  // GCC
#if defined(__INTEL_COMPILER)
#define ZEROERR_ICC ZEROERR_COMPILER(__INTEL_COMPILER / 100, __INTEL_COMPILER % 100, 0)
#endif  // ICC

#ifndef ZEROERR_MSVC
#define ZEROERR_MSVC 0
#endif  // ZEROERR_MSVC
#ifndef ZEROERR_CLANG
#define ZEROERR_CLANG 0
#endif  // ZEROERR_CLANG
#ifndef ZEROERR_GCC
#define ZEROERR_GCC 0
#endif  // ZEROERR_GCC
#ifndef ZEROERR_ICC
#define ZEROERR_ICC 0
#endif  // ZEROERR_ICC


// =================================================================================================
// == COMPILER WARNINGS HELPERS ====================================================================
// =================================================================================================

#if ZEROERR_CLANG && !ZEROERR_ICC
#define ZEROERR_PRAGMA_TO_STR(x)            _Pragma(#x)
#define ZEROERR_CLANG_SUPPRESS_WARNING_PUSH _Pragma("clang diagnostic push")
#define ZEROERR_CLANG_SUPPRESS_WARNING(w)   ZEROERR_PRAGMA_TO_STR(clang diagnostic ignored w)
#define ZEROERR_CLANG_SUPPRESS_WARNING_POP  _Pragma("clang diagnostic pop")
#define ZEROERR_CLANG_SUPPRESS_WARNING_WITH_PUSH(w) \
    ZEROERR_CLANG_SUPPRESS_WARNING_PUSH ZEROERR_CLANG_SUPPRESS_WARNING(w)
#else  // ZEROERR_CLANG
#define ZEROERR_CLANG_SUPPRESS_WARNING_PUSH
#define ZEROERR_CLANG_SUPPRESS_WARNING(w)
#define ZEROERR_CLANG_SUPPRESS_WARNING_POP
#define ZEROERR_CLANG_SUPPRESS_WARNING_WITH_PUSH(w)
#endif  // ZEROERR_CLANG

#if ZEROERR_GCC
#define ZEROERR_PRAGMA_TO_STR(x)          _Pragma(#x)
#define ZEROERR_GCC_SUPPRESS_WARNING_PUSH _Pragma("GCC diagnostic push")
#define ZEROERR_GCC_SUPPRESS_WARNING(w)   ZEROERR_PRAGMA_TO_STR(GCC diagnostic ignored w)
#define ZEROERR_GCC_SUPPRESS_WARNING_POP  _Pragma("GCC diagnostic pop")
#define ZEROERR_GCC_SUPPRESS_WARNING_WITH_PUSH(w) \
    ZEROERR_GCC_SUPPRESS_WARNING_PUSH ZEROERR_GCC_SUPPRESS_WARNING(w)
#else  // ZEROERR_GCC
#define ZEROERR_GCC_SUPPRESS_WARNING_PUSH
#define ZEROERR_GCC_SUPPRESS_WARNING(w)
#define ZEROERR_GCC_SUPPRESS_WARNING_POP
#define ZEROERR_GCC_SUPPRESS_WARNING_WITH_PUSH(w)
#endif  // ZEROERR_GCC

#if ZEROERR_MSVC
#define ZEROERR_MSVC_SUPPRESS_WARNING_PUSH __pragma(warning(push))
#define ZEROERR_MSVC_SUPPRESS_WARNING(w)   __pragma(warning(disable : w))
#define ZEROERR_MSVC_SUPPRESS_WARNING_POP  __pragma(warning(pop))
#define ZEROERR_MSVC_SUPPRESS_WARNING_WITH_PUSH(w) \
    ZEROERR_MSVC_SUPPRESS_WARNING_PUSH ZEROERR_MSVC_SUPPRESS_WARNING(w)
#else  // ZEROERR_MSVC
#define ZEROERR_MSVC_SUPPRESS_WARNING_PUSH
#define ZEROERR_MSVC_SUPPRESS_WARNING(w)
#define ZEROERR_MSVC_SUPPRESS_WARNING_POP
#define ZEROERR_MSVC_SUPPRESS_WARNING_WITH_PUSH(w)
#endif  // ZEROERR_MSVC

// =================================================================================================
// == COMPILER WARNINGS ============================================================================
// =================================================================================================

// both the header and the implementation suppress all of these,
// so it only makes sense to aggregate them like so
#define ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH                                                      \
    ZEROERR_CLANG_SUPPRESS_WARNING_PUSH                                                            \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wunknown-pragmas")                                            \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wweak-vtables")                                               \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wpadded")                                                     \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wmissing-prototypes")                                         \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wc++98-compat")                                               \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wc++98-compat-pedantic")                                      \
                                                                                                   \
    ZEROERR_GCC_SUPPRESS_WARNING_PUSH                                                              \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wunknown-pragmas")                                              \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wpragmas")                                                      \
    ZEROERR_GCC_SUPPRESS_WARNING("-Weffc++")                                                       \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wstrict-overflow")                                              \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wstrict-aliasing")                                              \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wmissing-declarations")                                         \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wuseless-cast")                                                 \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wnoexcept")                                                     \
                                                                                                   \
    ZEROERR_MSVC_SUPPRESS_WARNING_PUSH                                                             \
    /* these 4 also disabled globally via cmake: */                                                \
    ZEROERR_MSVC_SUPPRESS_WARNING(4514) /* unreferenced inline function has been removed */        \
    ZEROERR_MSVC_SUPPRESS_WARNING(4571) /* SEH related */                                          \
    ZEROERR_MSVC_SUPPRESS_WARNING(4710) /* function not inlined */                                 \
    ZEROERR_MSVC_SUPPRESS_WARNING(4711) /* function selected for inline expansion*/                \
    /* common ones */                                                                              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4616) /* invalid compiler warning */                             \
    ZEROERR_MSVC_SUPPRESS_WARNING(4619) /* invalid compiler warning */                             \
    ZEROERR_MSVC_SUPPRESS_WARNING(4996) /* The compiler encountered a deprecated declaration */    \
    ZEROERR_MSVC_SUPPRESS_WARNING(4706) /* assignment within conditional expression */             \
    ZEROERR_MSVC_SUPPRESS_WARNING(4512) /* 'class' : assignment operator could not be generated */ \
    ZEROERR_MSVC_SUPPRESS_WARNING(4127) /* conditional expression is constant */                   \
    ZEROERR_MSVC_SUPPRESS_WARNING(4820) /* padding */                                              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4625) /* copy constructor was implicitly deleted */              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4626) /* assignment operator was implicitly deleted */           \
    ZEROERR_MSVC_SUPPRESS_WARNING(5027) /* move assignment operator implicitly deleted */          \
    ZEROERR_MSVC_SUPPRESS_WARNING(5026) /* move constructor was implicitly deleted */              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4640) /* construction of local static object not thread-safe */  \
    ZEROERR_MSVC_SUPPRESS_WARNING(5045) /* Spectre mitigation for memory load */                   \
    ZEROERR_MSVC_SUPPRESS_WARNING(5264) /* 'variable-name': 'const' variable is not used */        \
    /* static analysis */                                                                          \
    ZEROERR_MSVC_SUPPRESS_WARNING(26439) /* Function may not throw. Declare it 'noexcept' */       \
    ZEROERR_MSVC_SUPPRESS_WARNING(26495) /* Always initialize a member variable */                 \
    ZEROERR_MSVC_SUPPRESS_WARNING(26451) /* Arithmetic overflow ... */                             \
    ZEROERR_MSVC_SUPPRESS_WARNING(26444) /* Avoid unnamed objects with custom ctor and dtor... */  \
    ZEROERR_MSVC_SUPPRESS_WARNING(26812) /* Prefer 'enum class' over 'enum' */

#define ZEROERR_SUPPRESS_COMMON_WARNINGS_POP \
    ZEROERR_CLANG_SUPPRESS_WARNING_POP       \
    ZEROERR_GCC_SUPPRESS_WARNING_POP         \
    ZEROERR_MSVC_SUPPRESS_WARNING_POP


#define ZEROERR_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_BEGIN                                 \
    ZEROERR_MSVC_SUPPRESS_WARNING_PUSH                                                             \
    ZEROERR_MSVC_SUPPRESS_WARNING(4548) /* before comma no effect; expected side - effect */       \
    ZEROERR_MSVC_SUPPRESS_WARNING(4265) /* virtual functions, but destructor is not virtual */     \
    ZEROERR_MSVC_SUPPRESS_WARNING(4986) /* exception specification does not match previous */      \
    ZEROERR_MSVC_SUPPRESS_WARNING(4350) /* 'member1' called instead of 'member2' */                \
    ZEROERR_MSVC_SUPPRESS_WARNING(4668) /* not defined as a preprocessor macro */                  \
    ZEROERR_MSVC_SUPPRESS_WARNING(4365) /* signed/unsigned mismatch */                             \
    ZEROERR_MSVC_SUPPRESS_WARNING(4774) /* format string not a string literal */                   \
    ZEROERR_MSVC_SUPPRESS_WARNING(4820) /* padding */                                              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4625) /* copy constructor was implicitly deleted */              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4626) /* assignment operator was implicitly deleted */           \
    ZEROERR_MSVC_SUPPRESS_WARNING(5027) /* move assignment operator implicitly deleted */          \
    ZEROERR_MSVC_SUPPRESS_WARNING(5026) /* move constructor was implicitly deleted */              \
    ZEROERR_MSVC_SUPPRESS_WARNING(4623) /* default constructor was implicitly deleted */           \
    ZEROERR_MSVC_SUPPRESS_WARNING(5039) /* pointer to pot. throwing function passed to extern C */ \
    ZEROERR_MSVC_SUPPRESS_WARNING(5045) /* Spectre mitigation for memory load */                   \
    ZEROERR_MSVC_SUPPRESS_WARNING(5105) /* macro producing 'defined' has undefined behavior */     \
    ZEROERR_MSVC_SUPPRESS_WARNING(4738) /* storing float result in memory, loss of performance */  \
    ZEROERR_MSVC_SUPPRESS_WARNING(5262) /* implicit fall-through */

#define ZEROERR_MAKE_STD_HEADERS_CLEAN_FROM_WARNINGS_ON_WALL_END ZEROERR_MSVC_SUPPRESS_WARNING_POP

#define ZEROERR_SUPPRESS_VARIADIC_MACRO \
    ZEROERR_CLANG_SUPPRESS_WARNING_WITH_PUSH("-Wgnu-zero-variadic-macro-arguments")

#define ZEROERR_SUPPRESS_VARIADIC_MACRO_POP ZEROERR_CLANG_SUPPRESS_WARNING_POP

#define ZEROERR_SUPPRESS_COMPARE                                          \
    ZEROERR_CLANG_SUPPRESS_WARNING_PUSH                                   \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wsign-conversion")                   \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wsign-compare")                      \
    ZEROERR_CLANG_SUPPRESS_WARNING("-Wgnu-zero-variadic-macro-arguments") \
    ZEROERR_GCC_SUPPRESS_WARNING_PUSH                                     \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wsign-conversion")                     \
    ZEROERR_GCC_SUPPRESS_WARNING("-Wsign-compare")                        \
    ZEROERR_MSVC_SUPPRESS_WARNING_PUSH                                    \
    ZEROERR_MSVC_SUPPRESS_WARNING(4388)                                   \
    ZEROERR_MSVC_SUPPRESS_WARNING(4389)                                   \
    ZEROERR_MSVC_SUPPRESS_WARNING(4018)

#define ZEROERR_SUPPRESS_COMPARE_POP                                    \
    ZEROERR_CLANG_SUPPRESS_WARNING_POP ZEROERR_GCC_SUPPRESS_WARNING_POP \
        ZEROERR_MSVC_SUPPRESS_WARNING_POP

#if ZEROERR_CLANG || ZEROERR_GCC
#define ZEROERR_UNUSED(x) x __attribute__((unused))
#elif defined(__LCLINT__)
#define ZEROERR_UNUSED(x) /*@unused@*/ x
#elif ZEROERR_MSVC
#define ZEROERR_UNUSED(x) \
    ZEROERR_MSVC_SUPPRESS_WARNING_WITH_PUSH(4100) x ZEROERR_MSVC_SUPPRESS_WARNING_POP
#else
#define ZEROERR_UNUSED(x) x
#endif
