#pragma once

#include "zeroerr/print.h"


namespace zeroerr {

struct Mark {};

template <typename format_str_t, typename... Args>
std::string format(format_str_t str, const Args&... args) {
    std::string result;
    return result;
}


}  // namespace zeroerr