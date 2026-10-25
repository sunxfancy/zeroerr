#pragma once
#include "zeroerr/print.h"

#include "zeroerr/string.h"
#include <utility>

namespace zeroerr {


/**
 * @brief Format a string with arguments
 * @param fmt The format string
 * @param args The arguments
 * @return zeroerr::String The formatted string
 * 
 * This function is used to format a string with arguments. The format string
 * is a string with placeholders in the form of `{}`. You can pass any type of
 * arguments to this function and it will format the string accordingly.
 * 
 * Example:
 *    format("Hello, {name}!", "John") -> "Hello, John!"
 *
 * Extra arguments are ignored. Too few arguments throw std::invalid_argument.
 * 
 */
template <typename... T>
ZEROERR_DIAGNOSTIC_NOINLINE zeroerr::String format(const char* fmt, T&&... args) {
    // The sentinel keeps the array valid for an empty parameter pack.
    const detail::DiagnosticValue values[] = {detail::makeDiagnosticValue(args)...,
                                              {nullptr, nullptr}};
    std::size_t ends[sizeof...(T) + 1];
    return detail::formatText(fmt, values, sizeof...(T), ends);
}

}  // namespace zeroerr
