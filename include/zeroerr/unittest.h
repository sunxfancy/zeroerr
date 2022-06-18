#pragma once

#include "zeroerr/config.h"

#include <iostream>
#include <set>
#include <string>

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

namespace detail {

extern std::set<TestCase>& getRegisteredTests();
struct regTest {
    regTest(const TestCase& tc) {
        // sort by filename and line number
        getRegisteredTests().insert(tc);
    }
};

}  // namespace detail

}  // namespace zeroerr
