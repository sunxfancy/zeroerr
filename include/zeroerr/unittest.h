#pragma once

#include "zeroerr/internal/config.h"

#include <cstring>
#include <string>
#include <vector>

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

    int add(TestContext&& local) {
        int type = 0;
        if (local.failed_as == 0 && local.warning_as == 0) {
            passed += 1;
        } else if (local.failed_as == 0) {
            warning += 1;
            type = 1;
        } else {
            failed += 1;
            type = 2;
        }
        passed_as += local.passed_as;
        warning_as += local.warning_as;
        failed_as += local.failed_as;

        memset(&local, 0, sizeof(local));
        return type;
    }
};

class UnitTest {
public:
    UnitTest& parseArgs(int argc, char** argv);
    int       run();
    bool      silent = true;
};

struct TestCase {
    std::string name;
    std::string file;
    unsigned    line;
    void (*func)(TestContext*);
    bool operator<(const TestCase& rhs) const;
};


template <typename T>
struct TestedObjects {
    void           add(T&& obj) { objects.push_back(std::forward<T>(obj)); }
    std::vector<T> objects;
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
    explicit regTest(const TestCase& tc);
};

struct regReporter {
    explicit regReporter(IReporter*);
};
}  // namespace detail


#pragma endregion


}  // namespace zeroerr
