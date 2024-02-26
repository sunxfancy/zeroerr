#pragma once
#include "zeroerr/format.h"
#include "zeroerr/internal/config.h"
#include "zeroerr/print.h"

#include <chrono>
#include <cstdlib>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

extern const char* ZEROERR_LOG_CATEGORY;

namespace std {
class mutex;
}

namespace zeroerr {

#pragma region log macros


#define ZEROERR_INFO(...)  ZEROERR_EXPAND(ZEROERR_INFO_(__VA_ARGS__))
#define ZEROERR_LOG(...)   ZEROERR_EXPAND(ZEROERR_LOG_(LOG_l, __VA_ARGS__))
#define ZEROERR_WARN(...)  ZEROERR_EXPAND(ZEROERR_LOG_(WARN_l, __VA_ARGS__))
#define ZEROERR_ERROR(...) ZEROERR_EXPAND(ZEROERR_LOG_(ERROR_l, __VA_ARGS__))
#define ZEROERR_FATAL(...) ZEROERR_EXPAND(ZEROERR_LOG_(FATAL_l, __VA_ARGS__))

#ifdef ZEROERR_USE_SHORT_LOG_MACRO

#ifdef INFO
#undef INFO
#endif

#ifdef LOG
#undef LOG
#endif

#ifdef WARN
#undef WARN
#endif

#ifdef ERR
#undef ERR
#endif

#ifdef FATAL
#undef FATAL
#endif

#ifdef VERBOSE
#undef VERBOSE
#endif

#define INFO(...)  ZEROERR_INFO(__VA_ARGS__)
#define LOG(...)   ZEROERR_LOG(__VA_ARGS__)
#define WARN(...)  ZEROERR_WARN(__VA_ARGS__)
#define ERR(...)   ZEROERR_ERROR(__VA_ARGS__)
#define FATAL(...) ZEROERR_FATAL(__VA_ARGS__)
#define VERBOSE(v) ZEROERR_VERBOSE(v)

#endif

#define ZEROERR_LOG_IF(condition, ACTION, ...) \
    do {                                       \
        if (condition) ACTION(__VA_ARGS__);    \
    } while (0)


#define INFO_IF(cond, ...)  ZEROERR_LOG_IF(cond, ZEROERR_INFO, __VA_ARGS__)
#define LOG_IF(cond, ...)   ZEROERR_LOG_IF(cond, ZEROERR_LOG, __VA_ARGS__)
#define WARN_IF(cond, ...)  ZEROERR_LOG_IF(cond, ZEROERR_WARN, __VA_ARGS__)
#define ERR_IF(cond, ...)   ZEROERR_LOG_IF(cond, ZEROERR_ERROR, __VA_ARGS__)
#define FATAL_IF(cond, ...) ZEROERR_LOG_IF(cond, ZEROERR_FATAL, __VA_ARGS__)


#define ZEROERR_LOG_EVERY_(n, ACTION, ...) \
    do {                                   \
        unsigned counter = 0;              \
        if (counter == 0) {                \
            counter = n;                   \
            ACTION(__VA_ARGS__);           \
        }                                  \
        --counter;                         \
    } while (0)


#define INFO_EVERY_(n, ...)  ZEROERR_LOG_EVERY_(n, ZEROERR_INFO, __VA_ARGS__)
#define LOG_EVERY_(n, ...)   ZEROERR_LOG_EVERY_(n, ZEROERR_LOG, __VA_ARGS__)
#define WARN_EVERY_(n, ...)  ZEROERR_LOG_EVERY_(n, ZEROERR_WARN, __VA_ARGS__)
#define ERR_EVERY_(n, ...)   ZEROERR_LOG_EVERY_(n, ZEROERR_ERROR, __VA_ARGS__)
#define FATAL_EVERY_(n, ...) ZEROERR_LOG_EVERY_(n, ZEROERR_FATAL, __VA_ARGS__)


#define ZEROERR_LOG_IF_EVERY_(n, cond, ACTION, ...) \
    do {                                            \
        unsigned counter = 0;                       \
        if (counter == 0 && (cond)) {               \
            counter = n;                            \
            ACTION(__VA_ARGS__);                    \
        }                                           \
        --counter;                                  \
    } while (0)

#define INFO_IF_EVERY_(n, cond, ...)  ZEROERR_LOG_IF_EVERY_(n, cond, ZEROERR_INFO, __VA_ARGS__)
#define LOG_IF_EVERY_(n, cond, ...)   ZEROERR_LOG_IF_EVERY_(n, cond, ZEROERR_LOG, __VA_ARGS__)
#define WARN_IF_EVERY_(n, cond, ...)  ZEROERR_LOG_IF_EVERY_(n, cond, ZEROERR_WARN, __VA_ARGS__)
#define ERR_IF_EVERY_(n, cond, ...)   ZEROERR_LOG_IF_EVERY_(n, cond, ZEROERR_ERROR, __VA_ARGS__)
#define FATAL_IF_EVERY_(n, cond, ...) ZEROERR_LOG_IF_EVERY_(n, cond, ZEROERR_FATAL, __VA_ARGS__)

#define ZEROERR_LOG_FIRST(cond, ACTION, ...) \
    do {                                     \
        bool first = true;                   \
        if (first && (cond)) {               \
            first = false;                   \
            ACTION(__VA_ARGS__);             \
        }                                    \
    } while (0)

#define INFO_FIRST(cond, ...)  ZEROERR_LOG_FIRST(cond, ZEROERR_INFO, __VA_ARGS__)
#define LOG_FIRST(cond, ...)   ZEROERR_LOG_FIRST(cond, ZEROERR_LOG, __VA_ARGS__)
#define WARN_FIRST(cond, ...)  ZEROERR_LOG_FIRST(cond, ZEROERR_WARN, __VA_ARGS__)
#define ERR_FIRST(cond, ...)   ZEROERR_LOG_FIRST(cond, ZEROERR_ERROR, __VA_ARGS__)
#define FATAL_FIRST(cond, ...) ZEROERR_LOG_FIRST(cond, ZEROERR_FATAL, __VA_ARGS__)

#define ZEROERR_LOG_FIRST_(n, cond, ACTION, ...) \
    do {                                         \
        unsigned counter = n;                    \
        if (n-- && (cond)) {                     \
            ACTION(__VA_ARGS__);                 \
        }                                        \
    } while (0)

#define INFO_FIRST_(n, cond, ...)  ZEROERR_LOG_FIRST_(n, cond, ZEROERR_INFO, __VA_ARGS__)
#define LOG_FIRST_(n, cond, ...)   ZEROERR_LOG_FIRST_(n, cond, ZEROERR_LOG, __VA_ARGS__)
#define WARN_FIRST_(n, cond, ...)  ZEROERR_LOG_FIRST_(n, cond, ZEROERR_WARN, __VA_ARGS__)
#define ERR_FIRST_(n, cond, ...)   ZEROERR_LOG_FIRST_(n, cond, ZEROERR_ERROR, __VA_ARGS__)
#define FATAL_FIRST_(n, cond, ...) ZEROERR_LOG_FIRST_(n, cond, ZEROERR_FATAL, __VA_ARGS__)

#ifdef _DEBUG
#define DLOG(ACTION, ...) ACTION(__VA_ARGS__)
#else
#define DLOG(ACTION, ...)
#endif

extern int _ZEROERR_G_VERBOSE;

#define ZEROERR_VERBOSE(v) if (zeroerr::_ZEROERR_G_VERBOSE >= (v)) 

#define ZEROERR_LOG_(severity, message, ...)                            \
    do {                                                                \
        ZEROERR_G_CONTEXT_SCOPE(true);                                  \
        auto msg = zeroerr::LogStream::getDefault().push(__VA_ARGS__);  \
                                                                        \
        static zeroerr::LogInfo log_info{                               \
            __FILE__, message, ZEROERR_LOG_CATEGORY,          __LINE__, \
            msg.size,    zeroerr::LogSeverity::severity};         \
        msg.log->info = &log_info;                                      \
        if (zeroerr::LogStream::getDefault().flush_mode ==              \
            zeroerr::LogStream::FlushMode::FLUSH_AT_ONCE)               \
            zeroerr::LogStream::getDefault().flush();                   \
    } while (0)

#define ZEROERR_INFO_(...) \
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

#ifdef ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER
#undef ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER
#endif

#define ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER(cond, level, ...) \
    ZEROERR_LOG_IF(cond, level, __VA_ARGS__)

#pragma endregion

namespace detail {

template <typename T, unsigned... I>
std::string gen_str(const char* msg, const T& args, seq<I...>) {
    return format(msg, std::get<I>(args)...);
}

template <typename T>
std::string gen_str(const char* msg, const T& args, seq<>) {
    return msg;
}

}  // namespace detail


enum LogSeverity {
    INFO_l,  // it will not write to file if no other log related
    LOG_l,
    WARN_l,
    ERROR_l,
    FATAL_l,  // it will contain a stack trace
};

struct LogTime {};

struct LogInfo {
    const char* filename;
    const char* message;
    const char* category;
    unsigned    line;
    unsigned    size;
    LogSeverity severity;
};

typedef void (*LogCustomCallback)(LogInfo);

extern void setLogLevel(LogSeverity level);
extern void setLogCategory(const char* categories);

struct LogMessage {
    LogMessage() { time = std::chrono::system_clock::now(); }

    virtual std::string str(bool colorful = true) = 0;

    // meta data of this log message
    LogInfo* info;

    // recorded wall time
    std::chrono::system_clock::time_point time;
};

#define zeroerr_color(x) (colorful ? x : "")

template <typename... T>
struct LogMessageImpl : LogMessage {
    LogMessageImpl(T... args) : LogMessage(), args(args...) {}

    std::string str(bool colorful = true) override {
        std::stringstream ss;
        std::time_t       t  = std::chrono::system_clock::to_time_t(time);
        std::tm           tm = *std::localtime(&t);

        ss << zeroerr_color(Dim) << '[' << zeroerr_color(Reset);
        switch (info->severity) {
            case INFO_l: ss << "INFO "; break;
            case LOG_l: ss << zeroerr_color(FgGreen) << "LOG  " << zeroerr_color(Reset); break;
            case WARN_l: ss << zeroerr_color(FgYellow) << "WARN " << zeroerr_color(Reset); break;
            case ERROR_l: ss << zeroerr_color(FgRed) << "ERROR" << zeroerr_color(Reset); break;
            case FATAL_l: ss << zeroerr_color(FgMagenta) << "FATAL" << zeroerr_color(Reset); break;
        }
        ss << " " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        std::string fileName(info->filename);

        auto p = fileName.find_last_of('/');
        if (p != std::string::npos) fileName = fileName.substr(p + 1);
        auto q = fileName.find_last_of('\\');
        if (q != std::string::npos) fileName = fileName.substr(q + 1);

        ss << " " << fileName << ":" << info->line;
        ss << zeroerr_color(Dim) << ']' << zeroerr_color(Reset) << "  "
           << gen_str(info->message, args, detail::gen_seq<sizeof...(T)>{});
        return ss.str();
    }

    std::tuple<T...> args;
};


constexpr size_t LogStreamMaxSize = 1024 * 1024 - 16;

struct DataBlock {
    char       data[LogStreamMaxSize];
    size_t     size = 0;
    DataBlock* next = nullptr;
};

class Logger {
public:
    virtual void flush(DataBlock*) = 0;
};

class LogStream {
public:
    LogStream();
    virtual ~LogStream();

    struct PushResult {
        LogMessage* log;
        unsigned    size;
    };

    enum FlushMode { FLUSH_AT_ONCE, FLUSH_WHEN_FULL, FLUSH_MANUALLY };
    enum LogMode { ASYNC, SYNC };
    enum DirMode {
        SINGLE_FILE       = 0,
        DAILY_FILE        = 1,
        SPLIT_BY_SEVERITY = 1 << 1,
        SPLIT_BY_CATEGORY = 1 << 2
    };

    template <typename... T>
    PushResult push(T&&... args) {
        unsigned    size = sizeof(LogMessageImpl<T...>);
        void*       p    = alloc_block(size);
        LogMessage* msg  = new (p) LogMessageImpl<T...>(std::forward<T>(args)...);
        return {msg, size};
    }

    void flush();

    void setBinFileLogger(std::string name);
    void setFileLogger(std::string name);
    void setStdoutLogger();
    void setStderrLogger();


    static LogStream& getDefault();
    FlushMode         flush_mode = FLUSH_AT_ONCE;
    LogMode           log_mode   = SYNC;
    DirMode           dir_mode   = SINGLE_FILE;

private:
    DataBlock *first, *last;
    Logger*    logger = nullptr;
#ifndef ZEROERR_NO_THREAD_SAFE
    std::mutex* mutex;
#endif
    void* alloc_block(unsigned size);
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
