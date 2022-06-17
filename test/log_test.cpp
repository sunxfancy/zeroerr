#define ZEROERR_ENABLE_PFR
#include "zeroerr/log.h"
#include "zeroerr/assert.h"
#include "zeroerr/dbg.h"
#include "zeroerr/print.h"
#include "zeroerr/unittest.h"

using namespace zeroerr;

TEST_CASE("log_test") {
    LOG(INFO) << "Hello";
    LOG(WARN) << "Test Warning";
}