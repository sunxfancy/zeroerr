#include "zeroerr/log.h"


namespace zeroerr {

LogMessage::LogMessage(const char* file, unsigned line, LogSeverity severity)
    : m_stream(new char[1024], 1024, 0) {}

LogMessage::~LogMessage() { flush(); }

void LogMessage::flush() {}

}  // namespace zeroerr
