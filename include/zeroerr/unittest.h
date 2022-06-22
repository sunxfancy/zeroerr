#pragma once

#include "zeroerr/config.h"

#include <string>

#pragma region unittest

#define ZEROERR_CREATE_TEST_FUNC(function, name)                     \
    static void                     function(zeroerr::TestContext*); \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(   \
        {name, __FILE__, __LINE__, function});                       \
    static void function(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT)

#define TEST_CASE(name) ZEROERR_CREATE_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)

#define SUB_CASE(name)

namespace zeroerr {

class TestContext {
public:
    unsigned passed = 0, warning = 0, failed = 0, skipped = 0;
    unsigned passed_as = 0, warning_as = 0, failed_as = 0, skipped_as = 0;
};

class UnitTest {
public:
    UnitTest& parseArgs(int argc, char** argv);
    int       run();
};

struct TestCase {
    std::string name;
    std::string file;
    unsigned    line;
    void (*func)(TestContext*);
    bool operator<(const TestCase& rhs) const;
};


#pragma endregion

#pragma region reporter

class IReporter {
public:
    virtual std::string getName() const = 0;

    virtual void reportQuery() = 0;

    virtual void reportResult(const TestContext& tc) = 0;

    // There are a list of events
    virtual void testStart() = 0;
    virtual void testEnd()   = 0;

    virtual void testCaseStart(const TestCase& tc) = 0;
    virtual void testCaseEnd(const TestCase& tc)   = 0;
};


namespace detail {

struct regTest {
    regTest(const TestCase& tc);
};

struct regReporter {
    regReporter(IReporter*);
};
}  // namespace detail


#pragma endregion


}  // namespace zeroerr
