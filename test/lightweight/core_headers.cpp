#include "forbidden_headers.h"
#include "zeroerr.h"


void core_headers() {
    zeroerr::Printer p;
    p(1, "hello");
    auto text = p.str();
    (void)text;
}
