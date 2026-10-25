#include <stdexcept>

#include "zeroerr/print.h"
#include <iostream>

namespace zeroerr {


Printer& getStdoutPrinter() {
    static Printer printer(std::cout);
    return printer;
}

Printer& getStderrPrinter() {
    static Printer printer(std::cerr);
    return printer;
}

}  // namespace zeroerr
#ifdef __GNUG__
#include <cxxabi.h>
#endif
namespace zeroerr {
String Printer::str() const {
    if (!use_stringstream) throw std::runtime_error("Printer is not capturing text");
    return os.str();
}
String Printer::demangle(const char* name) {
#ifdef __GNUG__
    int status = -4;
    char* result = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    String text(status == 0 ? result : name);
    std::free(result);
    return text;
#else
    return name;
#endif
}
}
