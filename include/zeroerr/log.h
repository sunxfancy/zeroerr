#pragma once

#include "zeroerr/config.h"


namespace zeroerr {

#define ZEROERR_LOG_INFO
#define ZEROERR_LOG_WARN
#define ZEROERR_LOG_ERROR
#define ZEROERR_LOG_FATAL


#define LOG(severity)  ZEROERR_LOG(severity).stream()
#define DLOG(severity) ZEROERR_DLOG(severity).stream()
#define VLOG(level)    ZEROERR_VLOG(level).stream()

#define LOG_IF(severity, condition)
#define DLOG_IF(severity, condition)
#define VLOG_IF(level, condition)

#define LOG_EVERY_N(severity, n)
#define DLOG_EVERY_N(severity, n)
#define VLOG_EVERY_N(level, n)

#define LOG_IF_EVERY_N(severity, n, condition)
#define DLOG_IF_EVERY_N(severity, n, condition)
#define VLOG_IF_EVERY_N(level, n, condition)


#define ZEROERR_INFO(...)                                                     \
    ZEROERR_INFO_IMPL(ZEROERR_NAMEGEN(_capture_), ZEROERR_NAMEGEN(_capture_), \
                      ZEROERR_NAMEGEN(_capture_name_), __VA_ARGS__)

#define ZEROERR_INFO_IMPL(mb_name, v_name, s_name, ...)                 \
    auto v_name = zeroerr::MakeContextScope([&](std::ostream* s_name) { \
        zeroerr::MessageBuilder mb_name(__FILE__, __LINE__);            \
        mb_name.m_stream = s_name;                                      \
        mb_name* __VA_ARGS__;                                           \
    })


struct LogTime {};

struct LogInfo {
    const char* severity;
    const char* filename;
    unsigned    line;
    unsigned    thread_id;
    LogTime     time;
};

typedef void (*LogCustomCallback)(LogInfo);


}  // namespace zeroerr
