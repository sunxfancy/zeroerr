#include "zeroerr/assert.h"
#include "zeroerr/fuzztest.h"
#include "zeroerr/unittest.h"

#include <string>

using namespace zeroerr;

FUZZ_TEST_CASE("fuzz_test") {
    FUZZ_FUNC([=](int k, std::string num) {
        int t = atoi(num.c_str());
        REQUIRE(k == t);
    })
        .WithDomains(InRange<int>(0, 10), Arbitrary<std::string>())
        .WithSeeds({{5, "Foo"}, {10, "Bar"}})
        .Run();
}