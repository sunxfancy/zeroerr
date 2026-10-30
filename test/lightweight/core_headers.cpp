// clang-format off: the guard must run after the library include.
#include "zeroerr.h"
#define ZEROERR_TEST_CORE_USES_MEMORY
#include "forbidden_headers.h"
// clang-format on


void core_headers() {
    zeroerr::Printer p;
    p(1, "hello");
    auto text = p.str();
    (void)text;
}
