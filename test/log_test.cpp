#define ZEROERR_ENABLE_PFR

#include "zeroerr/log.h"
#include "zeroerr/assert.h"
#include "zeroerr/benchmark.h"
#include "zeroerr/unittest.h"

#include "spdlog/spdlog.h"

using namespace zeroerr;

TEST_CASE("log_test") {
    LOG(INFO) << "Hello";
    LOG(WARN) << "Test Warning";
}


TEST_CASE("lazy evaluation") {
    int sum = 0;
    INFO("sum =", sum);
    for (int i = 1; i <= 10; ++i) {
        sum += i;
        INFO("i =", i);
        INFO("i+1 =", i + 1);
        REQUIRE(sum < 50);
    }
}


TEST_CASE("speed test") {
    Benchmark bench("log speed test");
    bench
        .run("stringstream",
             [] {
                 std::stringstream ss;
                 ss << "hello world\n";
                 doNotOptimizeAway(ss);
             })
        .run("log", [] { LOG(INFO) << "hello world\n"; })
        .run("spdlog", [] { spdlog::info("hello world\n"); })
        .report();
}

TEST_CASE("log group") {
    
    for ()

}