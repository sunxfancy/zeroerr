#include "zeroerr/log.h"


namespace zeroerr {

thread_local std::vector<IContextScope*> _ZEROERR_G_CONTEXT_SCOPE_VECTOR;

class Logger {
public:
};


class FileLogger : public Logger {};

class StderrLogger : public Logger {};


LogMessage::LogMessage(const char* file, unsigned line, LogSeverity severity)
    : m_stream(new char[1024], 1024, 0) {}

LogMessage::~LogMessage() { flush(); }

void LogMessage::flush() {}


}  // namespace zeroerr
