#pragma once
#include "zeroerr/config.h"

#include "zeroerr/color.h"
#include "zeroerr/print.h"

#include <iostream>
#include <tuple>  // for std::get and std::tie

#ifndef ZEROERR_DISABLE_DBG_MARCO
#define dbg(...) zeroerr::DebugExpr(__FILE__, __LINE__, __func__, #__VA_ARGS__, __VA_ARGS__)
#else
#define dbg(...) (__VA_ARGS__)
#endif

namespace zeroerr {

template <class T1, class... T>
struct last {
    using type = typename last<T...>::type;
};

template <class T1>
struct last<T1> {
    using type = T1;
};

template <typename... Args>
auto get_last(Args&&... args) -> typename last<Args...>::type {
    return std::get<sizeof...(Args) - 1>(std::tie(args...));
}

template <typename... T>
auto DebugExpr(const char* file, unsigned line, const char* func, const char* exprs, T... t) ->
    typename last<T...>::type {
    std::string fileName(file);
    auto        p = fileName.find_last_of('/');
    if (p != std::string::npos) fileName = fileName.substr(p + 1);

    std::cerr << Dim << "[" << fileName << ":" << line << " " << func << "] " << Reset;
    std::cerr << FgCyan << exprs << Reset << " = ";
    Printer print(std::cerr);
    print.line_break = "";
    print(t...);
    std::cerr << " (" << FgGreen;
    std::string typenames[] = {print.type(t)...};
    for (unsigned i = 0; i < sizeof...(T); ++i) {
        if (i != 0) std::cerr << ", ";
        std::cerr << typenames[i];
    }
    std::cerr << Reset << ")" << std::endl;
    return get_last(t...);
}


}  // namespace zeroerr
