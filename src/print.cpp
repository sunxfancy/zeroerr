#include "zeroerr/print.h"
#include <iostream>

namespace zeroerr {


extern Printer& getStdoutPrinter() {
    static Printer printer(std::cout);
    return printer;
}

extern Printer& getStderrPrinter() {
    static Printer printer(std::cerr);
    return printer;
}

}  // namespace zeroerr