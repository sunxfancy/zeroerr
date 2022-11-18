#define ZEROERR_ENABLE_PFR

#include "zeroerr/log.h"
#include "zeroerr/assert.h"
#include "zeroerr/benchmark.h"
#include "zeroerr/unittest.h"

#include "spdlog/spdlog.h"

using namespace zeroerr;


TEST_CASE("log_test") {
    LOG("Hello {i}", 1);
    WARN("Test Warning {print}", "print data");

    ERROR_IF(1 == 1, "1 == 1");
}


TEST_CASE("isempty") {
    int k = ISEMPTY(test, 1);
    // REQUIRE(k == 0);
}


TEST_CASE("lazy evaluation") {
    int sum = 0;
    INFO("sum =", sum);
    for (int i = 1; i <= 10; ++i) {
        sum += i;
        INFO("i =", i);
        INFO("i+1 =", i + 1);
        // REQUIRE(0 <= sum < 50);
    }
}


TEST_CASE("speed test") {
#ifdef ZEROERR_OS_UNIX
    uint64_t* data      = new uint64_t[1000000];
    FILE*     file      = fmemopen(data, 1000000 * sizeof(uint64_t), "w");
    FILE*     oldstdout = stdout;
    FILE*     oldstderr = stderr;
    stdout = stderr = file;
#endif
    Benchmark bench("log speed test");
    bench
        .run("stringstream",
             [] {
                 std::stringstream ss;
                 ss << "hello world " << 1.1;
                 doNotOptimizeAway(ss);
             })
        .run("log", [] { LOG("hello world {value}", 1.1); })
        .run("spdlog", [] { spdlog::info("hello world {:03.2f}", 1.1); })
        .report();
#ifdef ZEROERR_OS_UNIX
    stdout = oldstdout; stderr = oldstderr;
    fclose(file);
    delete[] data;
#endif
}

TEST_CASE("log group") {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += i;
        INFO("i = {i}", i);
        if (sum > 20) {
            LOG("sum = {sum}", sum);
        }
    }
}


TEST_CASE("debug log") {
    int sum = 0;
    // DLOG(LOG_FIRST, "debug log i = {i}", 1);
    // DLOG(WARNING_IF, sum < 5, "debug log i = {i}, sum = {sum}", 2, sum);
}