#pragma once
#include "zeroerr/internal/config.h"

// Thread safety support
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
