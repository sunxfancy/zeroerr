#include <ostream>
#include <string>
#include "zeroerr.hpp"
static_assert(std::is_same<decltype(zeroerr::format("{}", 1)), zeroerr::String>::value,
              "Include order must not change the core return type");
int other_printer_size() { return sizeof(zeroerr::Printer); }
