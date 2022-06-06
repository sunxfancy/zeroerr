#pragma once

#include "zeroerr/config.h"


namespace zeroerr
{



#define LOG(severity)   ZEROERR_LOG_##severity.stream()
#define SYSLOG(severity)   ZEROERR_SYSLOG_##severity(0).stream()





} // namespace zeroerr
