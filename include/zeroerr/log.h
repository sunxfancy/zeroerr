#pragma once
#include "zeroerr/config.h"
#include "zeroerr/print.h"

#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
namespace zeroerr {

#define ZEROERR_LOG(severity) \
    zeroerr::LogMessage(__FILE__, __LINE__, zeroerr::LogSeverity::severity)

#define LOG(severity) ZEROERR_LOG(severity).stream()

#ifdef _DEBUG
#define DLOG(severity) ZEROERR_LOG(severity).stream()
#else
#define DLOG(severity)
#endif

#define VLOG(v) \
    if (v <= zeroerr::LogLevel) ZEROERR_LOG(vlog).stream()

#define CLOG(severity, category) \
    if (category & zeroerr::LogCategory) ZEROERR_LOG(severity).stream()

#define LOG_IF(severity, condition) \
    if (condition) ZEROERR_LOG(severity).stream()

#define DLOG_IF(severity, condition)
#define VLOG_IF(level, condition)

#define LOG_EVERY_N(severity, n)
#define DLOG_EVERY_N(severity, n)
#define VLOG_EVERY_N(level, n)

#define LOG_IF_EVERY_N(severity, n, condition)
#define DLOG_IF_EVERY_N(severity, n, condition)
#define VLOG_IF_EVERY_N(level, n, condition)

#define INFO(...) ZEROERR_INFO(__VA_ARGS__)
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
    INFO,
    WARN,
    ERROR,
    FATAL,
};

struct LogTime {};

struct LogInfo {
    LogSeverity severity;
    const char* filename;
    unsigned    line;
    unsigned    thread_id;
};

typedef void (*LogCustomCallback)(LogInfo);

class LogStreamBuf : public std::streambuf {
public:
    // REQUIREMENTS: "len" must be >= 2 to account for the '\n' and '\0'.
    LogStreamBuf(char* buf, size_t len) { setp(buf, buf + len - 2); }

    // This effectively ignores overflow.
    int_type overflow(int_type ch) { return ch; }

    // Legacy public ostrstream method.
    size_t pcount() const { return static_cast<size_t>(pptr() - pbase()); }
    char*  pbase() const { return std::streambuf::pbase(); }
};

class LogStream : public std::ostream {
public:
    LogStream(char* buf, size_t len, size_t ctr)
        : std::ostream(NULL), streambuf(buf, len), ctr_(ctr), self_(this) {
        rdbuf(&streambuf);
    }

    size_t     ctr() const { return ctr_; }
    void       set_ctr(size_t ctr) { ctr_ = ctr; }
    LogStream* self() const { return self_; }

    // Legacy std::streambuf methods.
    size_t pcount() const { return streambuf.pcount(); }
    char*  pbase() const { return streambuf.pbase(); }
    char*  str() const { return pbase(); }

private:
    LogStream(const LogStream&)              = delete;
    LogStream&   operator=(const LogStream&) = delete;
    LogStreamBuf streambuf;
    size_t       ctr_;   // Counter hack (for the LOG_EVERY_X() macro)
    LogStream*   self_;  // Consistency check hack
};


struct LogMessage {
    LogMessage(const char* file, unsigned line, LogSeverity severity);
    ~LogMessage();
    std::ostream& stream() { return m_stream; }
    void          flush();

protected:
    LogStream m_stream;
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
