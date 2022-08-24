#include "zeroerr/log.h"


namespace zeroerr {

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

class Logger {
public:
};


class FileLogger : public Logger {};

class StderrLogger : public Logger {};

constexpr unsigned logbuffer_size = 65536;

struct LogSystem {
    char* temp_buf;
    unsigned temp_buf_size;

    bool mix_class = true;  
    bool mix_severity = true;

    Logger* logger = nullptr;

    LogSystem() {
        temp_buf = new char[logbuffer_size];
        temp_buf_size = logbuffer_size;
    }
};


LogSystem& getLogSystem() {
    static LogSystem inst;
    return inst;
}

Logger& getLogger() {
    return *(getLogSystem().logger);
}



LogMessage::LogMessage(const char* file, unsigned line, LogSeverity severity)
    : m_stream(getLogSystem().temp_buf, getLogSystem().temp_buf_size, 0) {}

LogMessage::~LogMessage() { flush(); }

void LogMessage::flush() {}



}  // namespace zeroerr
