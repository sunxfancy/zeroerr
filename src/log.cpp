#include "zeroerr/log.h"


namespace zeroerr {

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

class Logger {
public:
};


class FileLogger : public Logger {};

class StderrLogger : public Logger {};

constexpr unsigned logbuffer_size = 65536;

static char data[logbuffer_size];

LogMessage::LogMessage(const char* file, unsigned line, LogSeverity severity)
    : m_stream(data, logbuffer_size, 0) {}

LogMessage::~LogMessage() { flush(); }

void LogMessage::flush() {}


}  // namespace zeroerr
