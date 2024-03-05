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
        LOG("k: {k}, num:{num}, t: {t}", k, num, t);
        REQUIRE(k == t);
    })
        .WithDomains(InRange<int>(0, 10), Arbitrary<std::string>())
        .WithSeeds({{5, "Foo"}, {10, "Bar"}})
        .Run(1000);
}


FUZZ_TEST_CASE("fuzz_test2") {
    LOG("Run fuzz_test2");
    FUZZ_FUNC([=](int k, std::string num) {
        int t = atoi(num.c_str());
        LOG("k: {k}, t: {t}", k, t);
    })
        .WithDomains(InRange<int>(0, 10), Arbitrary<std::string>())
        .WithSeeds({{5, "Foo"}, {10, "Bar"}})
        .Run(100);
}

TEST_CASE("fuzz_serialize") { 
    LOG("fuzz_serialize"); 

    std::string data = R"({ "foo" 5 { 3.0f "test\sspace" } "bar" "hello" })";

    IRObject obj = IRObject::FromString(data);
    std::string str = obj.ToString(obj);

    LOG("data: {str}", str);

    CHECK(data == str);
}