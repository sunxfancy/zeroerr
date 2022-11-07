#pragma once

#include <string>
#include <sstream>
#include "zeroerr/print.h"

namespace zeroerr
{

template <typename... T>
std::string format(const char* fmt, T... args) {
    std::stringstream ss;
    bool parse_name = false;
    Printer print; print.isQuoted = false;
    std::string str_args[] = {print(args)...};
    int j = 0;
    for (const char* i = fmt; *i != '\0'; i++) {
        switch (*i)
        {
        case '{': 
            parse_name = true; 
            break;
        case '}': 
            parse_name = false; 
            ss << str_args[j++];
            break;
        default:
            if (!parse_name) ss << *i;
            break;
        }
    }
    return ss.str();
}

} // namespace zeroerr
