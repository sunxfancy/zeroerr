#include <zeroerr/unittest.h>
struct SecondFixture { char padding[128] = {}; int value = 91; };
TEST_CASE_FIXTURE(SecondFixture, "native.fixture.second") { REQUIRE(value == 91); }
