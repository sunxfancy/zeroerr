#include "zeroerr/fuzztest.h"

#include "zeroerr/log.h"
#include "zeroerr/assert.h"
#include "zeroerr/unittest.h"

#include <string>

using namespace zeroerr;

FUZZ_TEST_CASE("fuzz_test") {
    LOG("Run fuzz_test");
    FUZZ_FUNC([=](int k, std::string num) {
        int t = atoi(num.c_str());
        REQUIRE(k == t);
    })
        .WithDomains(InRange<int>(0, 10), Arbitrary<std::string>())
        .WithSeeds({{5, "Foo"}, {10, "Bar"}})
        .Run();
}

TEST_CASE("fuzz_serialize") { 
    LOG("fuzz_serialize"); 

    std::string data = R"({ "foo" 5 { 3.0f "test\sspace" } "bar" "hello" })";

    IRObject obj = IRObject::FromString(data);
    std::string str = obj.ToString(obj);

    LOG("data: {str}", str);

    CHECK(data == str);
}