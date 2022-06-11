#pragma once

#include "zeroerr/config.h"


namespace zeroerr {


#define LOG(severity) ZEROERR_LOG_##severity.stream()

#define ZEROERR_INFO(...)                                                     \
    ZEROERR_INFO_IMPL(ZEROERR_NAMEGEN(_capture_), ZEROERR_NAMEGEN(_capture_), \
                      ZEROERR_NAMEGEN(_capture_name_), __VA_ARGS__)

#define ZEROERR_INFO_IMPL(mb_name, v_name, s_name, ...)                         \
    auto v_name = zeroerr::detail::MakeContextScope([&](std::ostream* s_name) { \
        doctest::detail::MessageBuilder mb_name(__FILE__, __LINE__);            \
        mb_name.m_stream = s_name;                                              \
        mb_name* __VA_ARGS__;                                                   \
    })


}  // namespace zeroerr
