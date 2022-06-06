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

// Thread safety support
#ifdef ZEROERR_NO_THREAD_SAFE
#define ZEROERR_MUTEX(x) 
#define ZEROERR_LOCK(x)
#else
#define ZEROERR_MUTEX(x) static std::mutex x;
#define ZEROERR_LOCK(x) std::lock_guard<std::mutex> lock(x);
#endif

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))  
#define ZEROERR_OS_UNIX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define ZEROERR_OS_WINDOWS
#else
#define ZEROERR_OS_UNKNOWN
#endif
