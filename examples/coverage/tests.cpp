#include "zeroerr/assert.h"
#include "zeroerr/unittest.h"

int sign(int);

TEST_CASE("positive") { CHECK(sign(1) == 1); }
TEST_CASE("another positive input") { CHECK(sign(100) == 1); }
TEST_CASE("negative") { CHECK(sign(-1) == -1); }
TEST_CASE("zero") { CHECK(sign(0) == 0); }
