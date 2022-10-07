#pragma once
#include "zeroerr/internal/config.h"
#include "zeroerr/print.h"

#include <chrono>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace zeroerr {


#define INFO(...)    ZEROERR_INFO(__VA_ARGS__)
#define LOG(...)     ZEROERR_LOG(LOG, __VA_ARGS__)
#define WARNING(...) ZEROERR_LOG(WARN, __VA_ARGS__)
#define ERROR(...)   ZEROERR_LOG(ERROR, __VA_ARGS__)
#define FATAL(...)   ZEROERR_LOG(FATAL, __VA_ARGS__)


#define ZEROERR_LOG_IF(condition, ACTION, ...) \
    do {                                       \
        if (condition) ACTION(__VA_ARGS__);    \
    } while (0)

#define INFO_IF(cond, ...)  ZEROERR_LOG_IF(cond, INFO, __VA_ARGS__)
#define LOG_IF(cond, ...)   ZEROERR_LOG_IF(cond, LOG, __VA_ARGS__)
#define WARN_IF(cond, ...)  ZEROERR_LOG_IF(cond, WARN, __VA_ARGS__)
#define ERROR_IF(cond, ...) ZEROERR_LOG_IF(cond, ERROR, __VA_ARGS__)
#define FATAL_IF(cond, ...) ZEROERR_LOG_IF(cond, FATAL, __VA_ARGS__)


#define ZEROERR_LOG_EVERY_(n, ACTION, ...) \
    do {                                   \
        unsigned counter = 0;              \
        if (counter == 0) {                \
            counter = n;                   \
            ACTION(__VA_ARGS__);           \
        }                                  \
        --counter;                         \
    } while (0)


#define INFO_EVERY_(cond, ...)  ZEROERR_LOG_EVERY_(cond, INFO, __VA_ARGS__)
#define LOG_EVERY_(cond, ...)   ZEROERR_LOG_EVERY_(cond, LOG, __VA_ARGS__)
#define WARN_EVERY_(cond, ...)  ZEROERR_LOG_EVERY_(cond, WARN, __VA_ARGS__)
#define ERROR_EVERY_(cond, ...) ZEROERR_LOG_EVERY_(cond, ERROR, __VA_ARGS__)
#define FATAL_EVERY_(cond, ...) ZEROERR_LOG_EVERY_(cond, FATAL, __VA_ARGS__)


#define ZEROERR_LOG_IF_EVERY_(n, cond, ACTION, ...) \
    do {                                            \
        unsigned counter = 0;                       \
        if (counter == 0 && (cond)) {               \
            counter = n;                            \
            ACTION(__VA_ARGS__);                    \
        }                                           \
        --counter;                                  \
    } while (0)

#define INFO_IF_EVERY_(n, cond, ...)  ZEROERR_LOG_IF_EVERY_(n, cond, INFO, __VA_ARGS__)
#define LOG_IF_EVERY_(n, cond, ...)   ZEROERR_LOG_IF_EVERY_(n, cond, LOG, __VA_ARGS__)
#define WARN_IF_EVERY_(n, cond, ...)  ZEROERR_LOG_IF_EVERY_(n, cond, WARN, __VA_ARGS__)
#define ERROR_IF_EVERY_(n, cond, ...) ZEROERR_LOG_IF_EVERY_(n, cond, ERROR, __VA_ARGS__)
#define FATAL_IF_EVERY_(n, cond, ...) ZEROERR_LOG_IF_EVERY_(n, cond, FATAL, __VA_ARGS__)


#ifdef _DEBUG
#define DLOG(severity) ZEROERR_LOG(severity).stream()
#else
#define DLOG(severity)
#endif


#define ZEROERR_LOG(severity, message, ...)                                                  \
    do {                                                                                     \
        static zeroerr::LogInfo log_info{__FILE__, __LINE__, zeroerr::LogSeverity::severity, \
                                         message};                                           \
        zeroerr::LogStream::getDefault().push(log_info, ##__VA_ARGS__);                      \
    } while (0)

#define ZEROERR_INFO(...) \
    ZEROERR_INFO_IMPL(ZEROERR_NAMEGEN(_capture_), ZEROERR_NAMEGEN(_capture_), __VA_ARGS__)

#define ZEROERR_INFO_IMPL(mb_name, v_name, ...)                                \
    auto v_name = zeroerr::MakeContextScope([&](std::ostream& _capture_name) { \
        Printer print(_capture_name);                                          \
        print.isQuoted = false;                                                \
        print(__VA_ARGS__);                                                    \
    })

#ifdef ZEROERR_G_CONTEXT_SCOPE
#undef ZEROERR_G_CONTEXT_SCOPE
#endif

#define ZEROERR_G_CONTEXT_SCOPE(x)                        \
    if (x.passed == false) {                              \
        for (auto* i : _ZEROERR_G_CONTEXT_SCOPE_VECTOR) { \
            i->str(std::cerr);                            \
        }                                                 \
    }

extern size_t LogLevel;
extern size_t LogCategory;

enum LogSeverity {
    INFO,  // it will not write to file if no other log related
    LOG,
    WARN,
    ERROR,
    FATAL,  // it will terminate the program
};

struct LogTime {};

struct LogInfo {
    const char* filename;
    unsigned    line;
    LogSeverity severity;
    const char* message;
};

typedef void (*LogCustomCallback)(LogInfo);


struct LogMessage {
    LogMessage(LogInfo& info) : info(info) { time = std::chrono::system_clock::now(); }

    virtual std::string str() = 0;

    // meta data of this log message
    LogInfo& info;

    // recorded wall time
    std::chrono::system_clock::time_point time;
};

template <typename... T>
struct LogMessageImpl : LogMessage {
    LogMessageImpl(LogInfo& info, T... args) : LogMessage(info), args(args...) {}

    std::string str() override {
        std::ostringstream oss;
        Printer            print(oss);
        print(info.message, args);
        return oss.str();
    }

    std::tuple<T...> args;
};


constexpr size_t LogStreamMaxSize = 1024 * 1024;

class LogStream {
public:
    LogStream() { first = last = new DataBlock(); }

    struct DataBlock {
        char       data[LogStreamMaxSize];
        size_t     size = 0;
        DataBlock* next = nullptr;
    };

    template <typename... T>
    void push(LogInfo& info, T&&... args) {
        size_t size = sizeof(LogMessageImpl<T...>);
        if (size > LogStreamMaxSize) {
            throw std::runtime_error("LogStream::push: size > LogStreamMaxSize");
        }
        if (last->size + size > LogStreamMaxSize) {
            last->next = new DataBlock();
            last       = last->next;
        }
        void* p = last->data + last->size;
        last->size += size;
        auto msg = new (p) LogMessageImpl<T...>(info, std::forward<T>(args)...);
        printf("%s", msg->str().c_str());
    }

    static LogStream& getDefault();

private:
    DataBlock* first;
    DataBlock* last;
};


/**
 * @brief ContextScope is a helper class created in each basic block where you use INFO().
 * The context scope can has lazy evaluated function F(std::ostream&) that is called when the
 * assertation is failed
 */
class IContextScope {
public:
    virtual void str(std::ostream& os) const = 0;
};

extern thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

template <typename F>
class ContextScope : public IContextScope {
public:
    ContextScope(F f) : f_(f) { _ZEROERR_G_CONTEXT_SCOPE_VECTOR.push_back(this); }
    ~ContextScope() { _ZEROERR_G_CONTEXT_SCOPE_VECTOR.pop_back(); }

    virtual void str(std::ostream& os) const override { return f_(os); }

protected:
    F f_;
};

template <typename F>
ContextScope<F> MakeContextScope(const F& f) {
    return ContextScope<F>(f);
}


}  // namespace zeroerr
