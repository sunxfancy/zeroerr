#include "zeroerr/internal/threadsafe.h"
#ifndef ZEROERR_NO_THREAD_SAFE
#include <mutex>
namespace zeroerr {
namespace detail {
Mutex::Mutex() : value_(new std::mutex) {}
Mutex::~Mutex() { delete static_cast<std::mutex*>(value_); }
void Mutex::lock() { static_cast<std::mutex*>(value_)->lock(); }
void Mutex::unlock() { static_cast<std::mutex*>(value_)->unlock(); }
}  // namespace detail
}  // namespace zeroerr
#endif
