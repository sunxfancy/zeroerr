#include <zeroerr/unittest.h>
struct FirstFixture { int value = 17; };
TEST_CASE_FIXTURE(FirstFixture, "native.fixture.first") { REQUIRE(value == 17); }
