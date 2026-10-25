#define ZEROERR_ENABLE_BENCHMARK
#define ZEROERR_ENABLE_FUZZTEST
#define ZEROERR_ENABLE_TABLE
#define ZEROERR_ENABLE_STD_ADAPTER
#define ZEROERR_NO_MAIN
#define ZEROERR_DISABLE_AUTO_INIT
#define ZEROERR_IMPLEMENTATION
#include <complex>
#include "zeroerr.hpp"
int main() {
    zeroerr::Table table("optional");
    table.set_header({"value"});
    table.add_row({"ready"});
    if (table.str().find("ready") == zeroerr::String::npos) return 1;
    zeroerr::Benchmark                  benchmark("enabled");
    zeroerr::Arbitrary<zeroerr::String> domain;
    if (domain.GetValue(std::vector<char>{'a', 'b'}) != "ab") return 2;
    zeroerr::Printer printer;
    printer.isCompact  = true;
    printer.line_break = "";
    if (printer(std::complex<double>(1, 2)).str() != "(1+2i)") return 3;
    std::string compatible = zeroerr::format("{}", "legacy");
    if (compatible != "legacy") return 4;
    return 0;
}
