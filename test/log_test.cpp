#define ZEROERR_ENABLE_PFR

#include "zeroerr/log.h"
#include "zeroerr/assert.h"
#include "zeroerr/benchmark.h"
#include "zeroerr/unittest.h"

#include "spdlog/spdlog.h"

using namespace zeroerr;

TEST_CASE("log_test") {
    LOG("Hello {i}", 1);
    WARNING("Test Warning {print}", "print data");
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
    uint64_t* data      = new uint64_t[1000000];
    FILE*     file      = fmemopen(data, 1000000 * sizeof(uint64_t), "w");
    FILE*     oldstdout = stdout;
    stdout              = file;

    Benchmark bench("log speed test");
    bench
        .run("stringstream",
             [] {
                 std::stringstream ss;
                 ss << "hello world\n";
                 doNotOptimizeAway(ss);
             })
        .run("log", [] { LOG("hello world\n"); })
        .run("spdlog", [] { spdlog::info("hello world\n"); })
        .report();

    stdout = oldstdout;
    fclose(file);
    delete[] data;
}

TEST_CASE("log group") {}