#include "forbidden_headers.h"
#include "zeroerr.hpp"


#ifdef BENCHMARK
#error Benchmark must be explicitly enabled
#endif
#ifdef FUZZ_TEST_CASE
#error Fuzz tests must be explicitly enabled
#endif
int other_printer_size();
TEST_CASE("default core has no standard string or streams") {
    zeroerr::Printer print;
    print.isCompact  = true;
    print.line_break = "";
    CHECK(print(1, "hello").str() == "1 \"hello\"");
    CHECK(zeroerr::format("{} {}", 12, "value") == "12 value");
    CHECK(other_printer_size() == sizeof(zeroerr::Printer));
    LOG("lightweight {value}", 7);
    INFO("context", 12);
    CHECK(1 == 1);
}
