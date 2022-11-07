#pragma once
#include "zeroerr/internal/config.h"
#include "zeroerr/print.h"
#include "zeroerr/format.h"

#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace zeroerr {

#pragma region log macros


#define EXPAND(x)  x
#define INFO(...)  EXPAND(ZEROERR_INFO(__VA_ARGS__))
#define LOG(...)   EXPAND(ZEROERR_LOG(LOG, __VA_ARGS__))
#define WARN(...)  EXPAND(ZEROERR_LOG(WARN, __VA_ARGS__))
#define ERROR(...) EXPAND(ZEROERR_LOG(ERROR, __VA_ARGS__))
#define FATAL(...) EXPAND(ZEROERR_LOG(FATAL, __VA_ARGS__))


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

#define ZEROERR_LOG_FIRST(cond, ACTION, ...) \
    do {                                     \
        bool first = true;                   \
        if (first && (cond)) {               \
            first = false;                   \
            ACTION(__VA_ARGS__);             \
        }                                    \
    } while (0)

#define INFO_FIRST(cond, ...)  ZEROERR_LOG_FIRST(cond, INFO, __VA_ARGS__)
#define LOG_FIRST(cond, ...)   ZEROERR_LOG_FIRST(cond, LOG, __VA_ARGS__)
#define WARN_FIRST(cond, ...)  ZEROERR_LOG_FIRST(cond, WARN, __VA_ARGS__)
#define ERROR_FIRST(cond, ...) ZEROERR_LOG_FIRST(cond, ERROR, __VA_ARGS__)
#define FATAL_FIRST(cond, ...) ZEROERR_LOG_FIRST(cond, FATAL, __VA_ARGS__)

#define ZEROERR_LOG_FIRST_(n, cond, ACTION, ...) \
    do {                                         \
        unsigned counter = n;                    \
        if (n-- && (cond)) {                     \
            ACTION(__VA_ARGS__);                 \
        }                                        \
    } while (0)

#define INFO_FIRST_(n, cond, ...)  ZEROERR_LOG_FIRST_(n, cond, INFO, __VA_ARGS__)
#define LOG_FIRST_(n, cond, ...)   ZEROERR_LOG_FIRST_(n, cond, LOG, __VA_ARGS__)
#define WARN_FIRST_(n, cond, ...)  ZEROERR_LOG_FIRST_(n, cond, WARN, __VA_ARGS__)
#define ERROR_FIRST_(n, cond, ...) ZEROERR_LOG_FIRST_(n, cond, ERROR, __VA_ARGS__)
#define FATAL_FIRST_(n, cond, ...) ZEROERR_LOG_FIRST_(n, cond, FATAL, __VA_ARGS__)


#ifdef _DEBUG
#define DLOG(ACTION, ...) EXPAND(ACTION)(__VA_ARGS__)
#else
#define DLOG(ACTION, ...)
#endif


#define ZEROERR_LOG(severity, message, ...)                                               \
    do {                                                                                  \
        ZEROERR_G_CONTEXT_SCOPE(true);                                                    \
        auto*                   msg = zeroerr::LogStream::getDefault().push(__VA_ARGS__); \
        static zeroerr::LogInfo log_info{__FILE__, message, __LINE__, 0,                  \
                                         zeroerr::LogSeverity::severity};                 \
        msg->info = &log_info;                                                            \
        std::cerr << msg->str();                                                          \
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

#define ZEROERR_G_CONTEXT_SCOPE(x)                                 \
    if (x) {                                                       \
        for (auto* i : zeroerr::_ZEROERR_G_CONTEXT_SCOPE_VECTOR) { \
            i->str(std::cerr);                                     \
        }                                                          \
    }

#ifdef ZEROERR_PRINT_ASSERT_0
#undef ZEROERR_PRINT_ASSERT_0
#endif

#define ZEROERR_PRINT_ASSERT_0(cond, level, ...) ZEROERR_LOG_IF(cond, level, __VA_ARGS__)

#pragma endregion

namespace log_detail {

// Generate sequence of integers from 0 to N-1
// Usage: detail::gen_seq<N>  then use <size_t... I> to match it
template <std::size_t...>
struct seq {};

template <std::size_t N, std::size_t... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct gen_seq<0, Is...> : seq<Is...> {};

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
    const char* message;
    unsigned    line;
    unsigned    size;
    LogSeverity severity;
};

typedef void (*LogCustomCallback)(LogInfo);


struct LogMessage {
    LogMessage() { time = std::chrono::system_clock::now(); }

    virtual std::string str() = 0;

    // meta data of this log message
    LogInfo* info;

    // recorded wall time
    std::chrono::system_clock::time_point time;
};

template<typename T, std::size_t... I>
std::string gen_str(const char* msg, const T& args, log_detail::seq<I...>) {
    return format(msg, std::get<I>(args)...);
}

template<typename T>
std::string gen_str(const char* msg, const T& args, log_detail::seq<>) {
    return msg;
}


template <typename... T>
struct LogMessageImpl : LogMessage {
    LogMessageImpl(T... args) : LogMessage(), args(args...) {}

    std::string str() override {
        std::stringstream ss;
        std::time_t t = std::chrono::system_clock::to_time_t(time);
        std::tm tm = std::tm{0};
        gmtime_r(&t, &tm);

        ss << '[';
        switch (info->severity) {
            case INFO:
                ss << Dim << "INFO" << Reset;
                break;
            case LOG:
                ss << "LOG";
                break;
            case WARN:
                ss << FgYellow << "WARN" << Reset;
                break;
            case ERROR:
                ss << FgRed << "ERROR" << Reset;
                break;
            case FATAL:
                ss << FgMagenta << "FATAL" << Reset;
                break;
        }
        ss << " " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        std::string fileName(info->filename);
        auto        p = fileName.find_last_of('/');
        if (p != std::string::npos) fileName = fileName.substr(p + 1);

        ss << " " << fileName << ":" << info->line;
        ss << "] " << gen_str(info->message, args, log_detail::gen_seq<sizeof...(T)>{});
        return ss.str();
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
    LogMessage* push(T&&... args) {
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
        LogMessage* msg = new (p) LogMessageImpl<T...>(std::forward<T>(args)...);
        return msg;
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
