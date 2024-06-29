#pragma once
#include "zeroerr/print.h"

#include <sstream>
#include <string>

namespace zeroerr {


/**
 * @brief Format a string with arguments
 * @param fmt The format string
 * @param args The arguments
 * @return std::string The formatted string
 * 
 * This function is used to format a string with arguments. The format string
 * is a string with placeholders in the form of `{}`. You can pass any type of
 * arguments to this function and it will format the string accordingly.
 * 
 * Example:
 *    format("Hello, {name}!", "John") -> "Hello, John!"
 * 
 */
template <typename... T>
std::string format(const char* fmt, T... args) {
    std::stringstream ss;
    bool              parse_name = false;
    Printer           print;

    print.isQuoted         = false;
    print.isCompact        = true;
    print.line_break       = "";
    std::string str_args[] = {print(args)...};

    int j = 0;
    for (const char* i = fmt; *i != '\0'; i++) {
        switch (*i) {
            case '{': parse_name = true; break;
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

}  // namespace zeroerr
