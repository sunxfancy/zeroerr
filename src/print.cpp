#include "zeroerr/print.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#ifdef __GNUG__
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace zeroerr {


Printer::Printer() : os(*new std::stringstream()) { use_stringstream = true; }

void Printer::check_stream() {
    if (use_stringstream && clear_stream_before_printing) {
        auto& ss = static_cast<std::stringstream&>(os);
        ss.str(std::string());
        ss.clear();
    }
}

std::string Printer::str() const {
    if (!use_stringstream)
        throw std::runtime_error("Printer is not using stringstream");
    return static_cast<std::stringstream&>(os).str();
}

std::string Printer::demangle(const char* name) {
#ifdef __GNUG__
    int status = -4;
    char* res = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string ret = status == 0 ? res : name;
    std::free(res);
    return ret;
#else
    return name;
#endif
}

Printer& getStdoutPrinter() {
    static Printer printer(std::cout);
    return printer;
}

Printer& getStderrPrinter() {
    static Printer printer(std::cerr);
    return printer;
}

}  // namespace zeroerr