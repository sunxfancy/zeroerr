#include "zeroerr/fuzztest.h"

#include "zeroerr/assert.h"
#include "zeroerr/log.h"
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
        .Run(10);
}


FUZZ_TEST_CASE("fuzz_test2") {
    LOG("Run fuzz_test2");
    FUZZ_FUNC([=](int k, std::string num) {
        int t = atoi(num.c_str());
        LOG("k: {k}, t: {t}", k, t);
    })
        .WithDomains(InRange<int>(0, 10), Arbitrary<std::string>())
        .WithSeeds({{5, "Foo"}, {10, "Bar"}})
        .Run(10);
}


FUZZ_TEST_CASE("fuzz_test3") {
    LOG("Run fuzz_test3");
    FUZZ_FUNC([=](int k, std::vector<int> num) {
        int t = static_cast<int>(num.size());
        LOG("k: {k}, t: {t}", k, t);
    })
        .WithDomains(InRange<int>(0, 10), ContainerOf<std::vector>(Arbitrary<int>()))
        .Run(10);
}

FUZZ_TEST_CASE("fuzz_test4") {
    LOG("Run fuzz_test4");
    FUZZ_FUNC([=](int k, std::map<int, std::string> num) {
        int t = static_cast<int>(num.size());
        LOG("k: {k}, t: {t}", k, t);
    })
        .WithDomains(InRange<int>(0, 10), ContainerOf<std::map<int, std::string>>(
                                              PairOf(Arbitrary<int>(), Arbitrary<std::string>())))
        .Run(10);
}


FUZZ_TEST_CASE("fuzz_test4") {
    LOG("Run fuzz_test4");
    FUZZ_FUNC([=](int k, std::map<int, std::string> num) {
        int t = static_cast<int>(num.size());
        LOG("k: {k}, t: {t}", k, t);
    })
        .WithDomains(InRange<int>(0, 10), ContainerOf<std::map<int, std::string>>(
                                              PairOf(Arbitrary<int>(), Arbitrary<std::string>())))
        .Run(10);
}



int find_the_biggest(std::vector<int> vec) {
    if (vec.empty()) {
        WARN("Empty vector, vec.size() = {size}", vec.size());
        return 0;
    } 
    int max = 0;
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i] > max) {max = vec[i];} 
    }
    return max;
}

FUZZ_TEST_CASE("presentation") {
    LOG("Run fuzz_test");
    FUZZ_FUNC([=](std::vector<int> vec) {
        int ans = find_the_biggest(vec);
        // verify the result
        for (int i = 0; i < vec.size(); ++i) {
            CHECK(ans >= vec[i]);
        }
        if (vec.size() == 0) {
            CHECK(ans == 0);
            // verify WARN message to make sure the path is correct
            CHECK(LOG_GET(find_the_biggest, 
            "Empty vector, vec.size() = {size}", size, size_t) == 0);
        }
    })
        .WithDomains(ContainerOf<std::vector<int>>(
            InRange<int>(0, 100)))
        .WithSeeds({{{0, 1, 2, 3, 4, 5}}, {{1, 8, 4, 2, 3}}})
        .Run(100);
}

TEST_CASE("fuzz_serialize") {
    LOG("fuzz_serialize");

    std::string data = R"({ "foo" 5 { 3f "test\sspace" } "bar" "hello" })";

    IRObject    obj = IRObject::FromString(data);
    std::string str = obj.ToString(obj);

    LOG("data: {str}", str);

    CHECK(data == str);
}

TEST_CASE("fuzz_serialize_from_corpus") {
    using test_type = std::tuple<std::vector<std::tuple<int, std::string, float>>, std::string,
                                 std::map<int, float>>;
    test_type data  = {{{1, "foo", 3.0f}, {2, "bar", 4.0f}}, "hello", {{1, 3.0f}, {2, 4.0f}}};

    IRObject    obj = IRObject::FromCorpus(data);
    std::string str = obj.ToString(obj);
    LOG("data: {str}", str);

    auto data2 = IRObject::ToCorpus<test_type>(obj);

    CHECK(data == data2);
}