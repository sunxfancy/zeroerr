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

    std::string data = R"({ "foo" 5 { 3f "test\sspace" } "bar" "hello" })";

    IRObject obj = IRObject::FromString(data);
    std::string str = obj.ToString(obj);

    LOG("data: {str}", str);

    CHECK(data == str);
}

TEST_CASE("fuzz_serialize_from_corpus") {
    using test_type = std::tuple<std::vector<std::tuple<int, std::string, float>>, std::string, std::map<int, float>>;
    test_type data = {
        {{1, "foo", 3.0f}, {2, "bar", 4.0f}}, "hello", {{1, 3.0f}, {2, 4.0f}}
    };

    IRObject obj = IRObject::FromCorpus(data);
    std::string str = obj.ToString(obj);
    LOG("data: {str}", str);

    auto data2 = IRObject::ToCorpus<test_type>(obj);

    CHECK(data == data2);
}