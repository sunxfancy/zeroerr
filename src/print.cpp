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