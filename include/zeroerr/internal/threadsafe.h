#pragma once
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

#define ZEROERR_MUTEX(x)  static std::mutex x;
#define ZEROERR_LOCK(x)   std::lock_guard<std::mutex> lock(x);
#define ZEROERR_ATOMIC(x) std::atomic<x>
#define ZEROERR_LOAD(x)   x.load()

#include <atomic>
#include <mutex>

#endif
