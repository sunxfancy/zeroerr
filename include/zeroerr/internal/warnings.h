// !!This idea is got from the doctest library.!!
// =================================================================================================
// == COMPILER VERSION =============================================================================
// =================================================================================================

// ideas for the version stuff are taken from here: https://github.com/cxxstuff/cxx_detect
#pragma once

#ifdef _MSC_VER
#define ZEROERR_CPLUSPLUS _MSVC_LANG
#else
#define ZEROERR_CPLUSPLUS __cplusplus
#endif

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