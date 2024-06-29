#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/threadsafe.h"
#include "zeroerr/internal/typetraits.h"

#include "zeroerr/dbg.h"
#include "zeroerr/format.h"
#include "zeroerr/print.h"

#include <chrono>
#include <map>
#include <string>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

extern const char* ZEROERR_LOG_CATEGORY;

namespace std {
class mutex;
}

namespace zeroerr {

// clang-format off
#define ZEROERR_INFO(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_INFO_(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ZEROERR_LOG(...)   ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_LOG_(LOG_l, __VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ZEROERR_WARN(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_LOG_(WARN_l, __VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ZEROERR_ERROR(...) ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_LOG_(ERROR_l, __VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ZEROERR_FATAL(...) ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_LOG_(FATAL_l, __VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
// clang-format on

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

#define LOG_GET(func, id, name, type) ZEROERR_LOG_GET(func, id, name, type)

#endif  // ZEROERR_USE_SHORT_LOG_MACRO

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
#define DLOG(ACTION, ...) ZEROERR_EXPAND(ACTION(__VA_ARGS__))
#else
#define DLOG(ACTION, ...)
#endif

extern int _ZEROERR_G_VERBOSE;

#define ZEROERR_VERBOSE(v) if (zeroerr::_ZEROERR_G_VERBOSE >= (v))

#define ZEROERR_LOG_(severity, message, ...)                                           \
    do {                                                                               \
        ZEROERR_G_CONTEXT_SCOPE(true);                                                 \
        auto msg = zeroerr::log(__VA_ARGS__);                                          \
                                                                                       \
        static zeroerr::LogInfo log_info{__FILE__,                                     \
                                         __func__,                                     \
                                         message,                                      \
                                         ZEROERR_LOG_CATEGORY,                         \
                                         __LINE__,                                     \
                                         msg.size,                                     \
                                         zeroerr::LogSeverity::severity};              \
        msg.log->info = &log_info;                                                     \
        if (msg.stream.getFlushMode() == zeroerr::LogStream::FlushMode::FLUSH_AT_ONCE) \
            msg.stream.flush();                                                        \
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


// This macro can access the log in memory
#define ZEROERR_LOG_GET(func, id, name, type) \
    zeroerr::LogStream::getDefault().getLog<type>(#func, id, #name)


namespace detail {

template <typename T, unsigned... I>
std::string gen_str(const char* msg, const T& args, seq<I...>) {
    return format(msg, std::get<I>(args)...);
}

template <typename T>
std::string gen_str(const char* msg, const T&, seq<>) {
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

/**
 * @brief LogInfo is a struct to store the meta data of the log message.
 * @details LogInfo is a struct to store the meta data of the log message. 
 * It contains filename, function, message, category, line number, size, and severity.
 * Those data is initialized when the first log message is created using a static
 * local variable in the function where the log message is put.
 * 
 * For example:
 *   void foo() {
 *     log("Hello, {name}!", "John");
 *   }
 * 
 * The inner implementation could be considered as (not exactly 
 * since message is allocated from a pool):
 *   void foo() {
 *      static LogInfo log_info{ 
 *          __FILE__, __func__, "Hello, {name}!", 
 *          ZEROERR_LOG_CATEGORY, 
 *          __LINE__, 
 *          sizeof("Hello, world!"), 
 *          LogSeverity::INFO_l);
 *      LogMessage* logdata = new LogMessageImpl<std::string>("John");
 *      logdata->info = &log_info;
 *    }
 */
struct LogInfo {
    const char*                filename;
    const char*                function;
    const char*                message;
    const char*                category;
    unsigned                   line;
    unsigned                   size;
    LogSeverity                severity;
    std::map<std::string, int> names;

    LogInfo(const char* filename, const char* function, const char* message, const char* category,
            unsigned line, unsigned size, LogSeverity severity);
};

struct LogMessage;
typedef std::string (*LogCustomCallback)(const LogMessage&, bool colorful);

/**
 * @brief set the log level
 */
extern void setLogLevel(LogSeverity level);

/**
 * @brief set the log category
 */
extern void setLogCategory(const char* categories);

/**
 * @brief set the log custom callback, this can support custom format of the log message
 */
extern void setLogCustomCallback(LogCustomCallback callback);

/**
 * @brief suspend the log to flush to the file
 */
extern void suspendLog();

/**
 * @brief resume the log to flush to the file
 */
extern void resumeLog();

/**
 * @brief LogMessage is a class to store the log message.
 * @details LogMessage is a class to store the log message and a base class
 * for all the messages implementation. You can create a log message with any
 * type of arguments and it will store the arguments in a tuple.
 * The log message can be converted to a string with the str() function.
 * You can also get the raw pointer of the arguments with the getRawLog() function.
 */
struct LogMessage {
    // time is assigned when the log message is created
    LogMessage() { time = std::chrono::system_clock::now(); }

    // convert the log message to a string
    virtual std::string str() const = 0;

    // get the raw data pointer of the field with the name
    virtual void* getRawLog(std::string name) const = 0;

    // a map of the data indexing by the field name
    // for example: log("print {i}", 1);
    // a map of {"i": "1"} will be returned
    virtual std::map<std::string, std::string> getData() const = 0;

    // meta data of this log message
    const LogInfo* info;

    // recorded wall time
    std::chrono::system_clock::time_point time;
};


/**
 * @brief LogMessageImpl is the implementation of the LogMessage.
 * @details LogMessageImpl is the implementation of the LogMessage. It stores
 * the arguments in a tuple and provides the str() function to convert the log
 * message to a string. All fields could be accessed by getRawLog() or getData().
 */
template <typename... T>
struct LogMessageImpl final : LogMessage {
    std::tuple<T...> args;
    LogMessageImpl(T... args) : LogMessage(), args(args...) {}

    std::string str() const override {
        return gen_str(info->message, args, detail::gen_seq<sizeof...(T)>{});
    }

    // This is a helper class to get the raw pointer of the tuple
    struct GetTuplePtr {
        void* ptr = nullptr;
        template <typename H>
        void operator()(H& v) {
            ptr = (void*)&v;
        }
    };

    void* getRawLog(std::string name) const override {
        GetTuplePtr f;
        detail::visit_at(args, info->names.at(name), f);
        return f.ptr;
    }

    struct PrintTupleData {
        std::map<std::string, std::string> data;
        Printer                            print;
        std::string                        name;

        PrintTupleData() : print() {
            print.isCompact  = true;
            print.line_break = "";
        }

        template <typename H>
        void operator()(H& v) {
            data[name] = print(v);
        }
    };

    std::map<std::string, std::string> getData() const override {
        PrintTupleData printer;
        for (auto it = info->names.begin(); it != info->names.end(); ++it) {
            printer.name = it->first;
            detail::visit_at(args, it->second, printer);
        }
        return printer.data;
    }
};

struct DataBlock;
class LogStream;

class Logger {
public:
    virtual ~Logger()              = default;
    virtual void flush(DataBlock*) = 0;
};

struct PushResult {
    LogMessage* log;
    unsigned    size;
    LogStream&  stream;
};

/**
 * @brief LogIterator is a class to iterate the log messages.
 * @details LogIterator is a class to iterate the log messages. You can also filter
 * the log messages by message, function name, and line number.
 *
 * An example of using LogIterator:
 *    for (int i = 0; i < 10; ++i)
 *      log("i = {i}", i);
 *
 *    LogIterator it = LogStream::getDefault().begin("Hello, world!");
 *    LogIterator end = LogStream::getDefault().end();
 *
 *    for (; it != end; ++it) {
 *      LogMessage& msg = *it;
 *      std::cout << msg.str() << std::endl;   // "i = {i}"
 *      std::cout << it.get<int>("i") << std::endl; // "0", "1", "2", ...
 *    }
 */
class LogIterator {
public:
    LogIterator() : p(nullptr), q(nullptr) {}
    LogIterator(LogStream& stream, std::string message = "", std::string function_name = "",
                int line = -1);
    LogIterator(const LogIterator& rhs) : p(rhs.p), q(rhs.q) {}
    LogIterator& operator=(const LogIterator& rhs) {
        p = rhs.p;
        q = rhs.q;
        return *this;
    }

    LogIterator& operator++();
    LogIterator  operator++(int) {
        LogIterator tmp = *this;
        ++*this;
        return tmp;
    }

    template <typename T>
    T get(std::string name) {
        void* data = q->getRawLog(name);
        if (data) return *(T*)(data);
        return T{};
    }

    bool operator==(const LogIterator& rhs) const { return p == rhs.p && q == rhs.q; }
    bool operator!=(const LogIterator& rhs) const { return !(*this == rhs); }

    LogMessage& get() const { return *q; }
    LogMessage& operator*() const { return *q; }
    LogMessage* operator->() const { return q; }

    void check_at_safe_pos();

    friend class LogStream;

protected:
    bool check_filter();
    void next();

    DataBlock*  p;
    LogMessage* q;

    std::string function_name_filter;
    std::string message_filter;
    int         line_filter = -1;
};

/**
 * @brief LogStream is a class to manage the log messages.
 * @details LogStream is a class to manage the log messages. It can be used to
 * create log messages and push them to the logger. A default LogStream is
 * created when the first time you call getDefault() function (or first log happens).
 * You can also adjust the way to flush the messages and how the log messages are
 * written to the log file.
 */
class LogStream {
public:
    LogStream();
    virtual ~LogStream();

    enum FlushMode { FLUSH_AT_ONCE, FLUSH_WHEN_FULL, FLUSH_MANUALLY };
    enum LogMode { ASYNC, SYNC };
    enum DirMode {
        SINGLE_FILE       = 0,
        DAILY_FILE        = 1,
        SPLIT_BY_SEVERITY = 1 << 1,
        SPLIT_BY_CATEGORY = 1 << 2
    };


    /**
     * @brief push a log message to the stream
     * @tparam T The types of the arguments
     * @param args The arguments
     * @return PushResult The result of the push
     *
     * This function is used to push a log message to the stream. You can pass
     * any type of arguments to this function and it will create a log message
     * with the arguments. The log message is not written to the log file until
     * the stream is flushed.
     *
     * The log message is structured as a tuple of the arguments in the inner
     * implementation class LogMessageImpl. After the log message is created, it
     * used type erasure to return a LogMessage pointer to the caller.
     * 
     * The stored data type is determined by the to_store_type_t<T> template.
     * For all the string type in raw pointer like const char* or char[],
     * it will be converted to std::string.
     * All reference type (including right value reference) will be converted 
     * to the original type.
     */
    template <typename... T>
    PushResult push(T&&... args) {
        // unsigned size = sizeof(LogMessageImpl<T...>);
        unsigned size = sizeof(LogMessageImpl<detail::to_store_type_t<T>...>);
        void*    p;
        if (use_lock_free)
            p = alloc_block_lockfree(size);
        else
            p = alloc_block(size);
        // LogMessage* msg = new (p) LogMessageImpl<T...>(std::forward<T>(args)...);
        LogMessage* msg = new (p) LogMessageImpl<detail::to_store_type_t<T>...>(args...);
        return {msg, size, *this};
    }


    /**
     * @brief get a log message from the stream
     * @tparam T The type of the log message
     * @param func The function name of the log message
     * @param line The line number of the log message
     * @param name The name of field you want to get
     *
     * This function is used to get a log message from the stream and extract
     * the field with the name. The function will return the field with the type
     * T. However, this type must be specified by the caller.
     */
    template <typename T>
    T getLog(std::string func, unsigned line, std::string name) {
        void* data = getRawLog(func, line, name);
        if (data) return *(T*)(data);
        return T{};
    }

    /**
     * @brief get a log message from the stream
     * @tparam T The type of the log message
     * @param func The function name of the log message
     * @param msg The message of the log message
     * @param name The name of field you want to get
     *
     * This function is used to get a log message from the stream and extract
     * the field with the name. The function will return the field with the type
     * T. However, this type must be specified by the caller.
     */
    template <typename T>
    T getLog(std::string func, std::string msg, std::string name) {
        void* data = getRawLog(func, msg, name);
        if (data) return *(T*)(data);
        return T{};
    }

    LogIterator begin(std::string message = "", std::string function_name = "", int line = -1) {
        return LogIterator(*this, message, function_name, line);
    }
    LogIterator end() { return LogIterator(); }
    LogIterator current(std::string message = "", std::string function_name = "", int line = -1);

    void flush();
    void setFileLogger(std::string name, DirMode mode1 = SINGLE_FILE, DirMode mode2 = SINGLE_FILE,
                       DirMode mode3 = SINGLE_FILE);
    void setStdoutLogger();
    void setStderrLogger();

    static LogStream& getDefault();

    void setFlushAtOnce() { flush_mode = FLUSH_AT_ONCE; }
    void setFlushWhenFull() { flush_mode = FLUSH_WHEN_FULL; }
    void setFlushManually() { flush_mode = FLUSH_MANUALLY; }
    void setAsyncLog() { log_mode = ASYNC; }
    void setSyncLog() { log_mode = SYNC; }

    FlushMode getFlushMode() const { return flush_mode; }
    void      setFlushMode(FlushMode mode) { flush_mode = mode; }
    LogMode   getLogMode() const { return log_mode; }
    void      setLogMode(LogMode mode) { log_mode = mode; }

    bool use_lock_free = true;

    friend class LogIterator;

private:
    DataBlock *first, *prepare;
    ZEROERR_ATOMIC(DataBlock*) m_last;
    Logger*   logger     = nullptr;
    FlushMode flush_mode = FLUSH_AT_ONCE;
    LogMode   log_mode   = SYNC;
#ifndef ZEROERR_NO_THREAD_SAFE
    std::mutex* mutex;
#endif

    // The implementation of alloc objects by giving a size
    void* alloc_block(unsigned size);
    void* alloc_block_lockfree(unsigned size);

    // The implementation of getLog which returns a raw pointer
    // This way can reduce the overhead of code generation by template
    void* getRawLog(std::string func, unsigned line, std::string name);
    void* getRawLog(std::string func, std::string msg, std::string name);
};


template <typename... T>
PushResult log(T&&... args) {
    return LogStream::getDefault().push(std::forward<T>(args)...);
}

template <typename... T>
PushResult log(LogStream& stream, T&&... args) {
    return stream.push(std::forward<T>(args)...);
}


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

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP