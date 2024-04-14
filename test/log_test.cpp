#define ZEROERR_ENABLE_PFR
#define ZEROERR_ENABLE_SPEED_TEST

#include "zeroerr/log.h"
#include "zeroerr/assert.h"
#include "zeroerr/benchmark.h"
#include "zeroerr/unittest.h"

#ifdef ZEROERR_ENABLE_SPEED_TEST
#include "spdlog/spdlog.h"
#endif

using namespace zeroerr;


TEST_CASE("log_test") {
    LOG("Hello {i}", 1);
    WARN("Test Warning {print}", "print data");

    ERR_IF(1 == 1, "1 == 1");
}


TEST_CASE("isempty") {
    int k = ZEROERR_ISEMPTY(test, 1);
    REQUIRE(k == 0);
}


TEST_CASE("lazy evaluation") {
    int sum = 0;
    INFO("sum =", sum);
    for (int i = 1; i <= 10; ++i) {
        sum += i;
        INFO("i =", i);
        INFO("i+1 =", i + 1);
        REQUIRE(0 <= sum < 50);
    }
}
#ifdef ZEROERR_ENABLE_SPEED_TEST
BENCHMARK("speedtest") {
#ifdef ZEROERR_OS_UNIX
    uint64_t* data      = new uint64_t[1000000];
    FILE*     file      = fmemopen(data, 1000000 * sizeof(uint64_t), "w");
    FILE*     oldstdout = stdout;
    FILE*     oldstderr = stderr;
    stdout = stderr = file;
#endif
    zeroerr::LogStream::getDefault().flush_mode = zeroerr::LogStream::FlushMode::FLUSH_MANUALLY;
    std::stringstream ss;
    Benchmark         bench("log speed test");
    bench.run("spdlog", [] { spdlog::info("hello world {:03.2f}", 1.1); })
        .run("stringstream",
             [&] {
                 ss << "hello world " << 1.1;
                 doNotOptimizeAway(ss);
             })
        .run("log", [] { LOG("hello world {value}", 1.1); })
        .report();
    zeroerr::LogStream::getDefault().flush_mode = zeroerr::LogStream::FlushMode::FLUSH_AT_ONCE;

#ifdef ZEROERR_OS_UNIX
    stdout = oldstdout;
    stderr = oldstderr;
    fclose(file);
    delete[] data;
#endif
}
#endif

TEST_CASE("log group") {
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += i;
        INFO("i = ", i);
        if (sum > 20) {
            LOG("sum = {sum}", sum);
        }
    }
}


static void test_feature(int k) {
    if (k == 1) {
        LOG("k equals 1");
    }
}

TEST_CASE("cross function info") {
    int k = 0;
    INFO("k = ", k);

    test_feature(k);
    k = 1;
    test_feature(k);
    k = 2;
    test_feature(k);
}


TEST_CASE("debug log") {
    int sum = 0;
    DLOG(LOG_FIRST, sum < 5, "debug log i = {i}", 1);
    DLOG(WARN_IF, sum < 5, "debug log i = {i}, sum = {sum}", 2, sum);
}

TEST_CASE("log to file") {
    zeroerr::LogStream::getDefault().setFileLogger("log.txt");
    LOG("log to file {i}", 1);
    LOG("log the data {i}", 2);
    zeroerr::LogStream::getDefault().setStderrLogger();
}

TEST_CASE("verbose") {
    zeroerr::_ZEROERR_G_VERBOSE = 1;
    VERBOSE(1) LOG("verbose log {i}", 1);
    VERBOSE(2) LOG("verbose log {i}", 2);
}


static void function() {
    LOG("function log {i}", 1);
    LOG("function log {sum}, {i}", 10, 1);
}

TEST_CASE("access log in Test case") {
    zeroerr::suspendLog();
    function();
    std::cerr << LOG_GET(function, 122, i, int) << std::endl;
    std::cerr << LOG_GET(function, 123, sum, int) << std::endl;
    std::cerr << LOG_GET(function, 123, i, int) << std::endl;

    CHECK(LOG_GET(function, 122, i, int) == 1);
    CHECK(LOG_GET(function, 123, sum, int) == 9);
    CHECK(LOG_GET(function, 123, i, int) == 2);
    zeroerr::resumeLog();
}

TEST_CASE("access log in Test case") {
    zeroerr::suspendLog();
    function();
    std::cerr << LOG_GET(function, "function log {i}", i, int) << std::endl;
    std::cerr << LOG_GET(function, "function log {sum}, {i}", sum, int) << std::endl;
    std::cerr << LOG_GET(function, "function log {sum}, {i}", i, int) << std::endl;

    CHECK(LOG_GET(function, "function log {i}", i, int) == 1);
    CHECK(LOG_GET(function, "function log {sum}, {i}", sum, int) == 9);
    CHECK(LOG_GET(function, "function log {sum}, {i}", i, int) == 2);
    zeroerr::resumeLog();
}

TEST_CASE("multiple log stream") {
    zeroerr::LogStream stream1, stream2;
    stream1.setFileLogger("log1.txt");
    stream2.setFileLogger("log2.txt");

    LOG("log stream {i}", stream1, 1);
    LOG("log stream {i}", stream2, 2);
}

TEST_CASE("log to dir") {
    zeroerr::LogStream::getDefault().setFileLogger("./logdir", LogStream::SPLIT_BY_CATEGORY,
                                                   LogStream::SPLIT_BY_SEVERITY,
                                                   LogStream::DAILY_FILE);
    LOG("log to dir {i}", 1);
    WARN("warn log to dir {i}", 2);
    zeroerr::LogStream::getDefault().setStderrLogger();
}