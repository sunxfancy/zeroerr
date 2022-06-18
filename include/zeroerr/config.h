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
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define ZEROERR_OS_WINDOWS
#else
#define ZEROERR_OS_UNKNOWN
#endif


#if defined(NDEBUG) && !defined(ZEROERR_ALWAYS_ASSERT)
#define ZEROERR_NO_ASSERT
#endif

// This is used for generating a unique name based on the file name and line number
#define ZEROERR_CAT_IMPL(s1, s2) s1##s2
#define ZEROERR_CAT(x, s)        ZEROERR_CAT_IMPL(x, s)

#ifdef __COUNTER__
#define ZEROERR_NAMEGEN(x) ZEROERR_CAT(x, __COUNTER__)
#else  // __COUNTER__
#define ZEROERR_NAMEGEN(x) ZEROERR_CAT(x, __LINE__)
#endif  // __COUNTER__
