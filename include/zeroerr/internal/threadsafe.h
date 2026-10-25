#pragma once

// libc++ offers compatibility-only transitive includes through C++20.
// Suppress those while loading our explicit dependencies, then restore the
// caller's setting. This does not change standard-library types or ABI.
#pragma push_macro("_LIBCPP_REMOVE_TRANSITIVE_INCLUDES")
#ifndef _LIBCPP_REMOVE_TRANSITIVE_INCLUDES
#define _LIBCPP_REMOVE_TRANSITIVE_INCLUDES
#endif

#include "zeroerr/internal/config.h"

/**
 * @brief Thread safety support
 * This header provides thread-safe support for zeroerr.
 * 
 * It defines macros for mutexes, locks, and atomic operations.
 * The macros are conditionally defined based on the ZEROERR_NO_THREAD_SAFE flag.
 */
#ifdef ZEROERR_NO_THREAD_SAFE

#define ZEROERR_MUTEX(x)
#define ZEROERR_LOCK(x)
#define ZEROERR_ATOMIC(x) x
#define ZEROERR_LOAD(x)   x

#else

#define ZEROERR_MUTEX(x)  static zeroerr::detail::Mutex x;
#define ZEROERR_LOCK(x)   zeroerr::detail::LockGuard lock(x);
#define ZEROERR_ATOMIC(x) std::atomic<x>
#define ZEROERR_LOAD(x)   x.load()

#include <atomic>
namespace zeroerr { namespace detail {
class Mutex {
public:
    Mutex(); ~Mutex();
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    void lock(); void unlock();
private: void* value_;
};
class LockGuard {
public: explicit LockGuard(Mutex& mutex) : mutex_(mutex) { mutex_.lock(); }
    ~LockGuard() { mutex_.unlock(); }
    LockGuard(const LockGuard&) = delete;
private: Mutex& mutex_;
};
}}


#endif

#pragma pop_macro("_LIBCPP_REMOVE_TRANSITIVE_INCLUDES")
