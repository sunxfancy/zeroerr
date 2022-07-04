#pragma once

#include "zeroerr/internal/config.h"

#include <cstring>
#include <functional>
#include <string>
#include <vector>

#pragma region unittest

#define ZEROERR_CREATE_TEST_FUNC(function, name)                     \
    static void                     function(zeroerr::TestContext*); \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(   \
        {name, __FILE__, __LINE__, function});                       \
    static void function(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT)

#define TEST_CASE(name) ZEROERR_CREATE_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)

#define SUB_CASE(name)                                                   \
    zeroerr::SubCaseReg(name, __FILE__, __LINE__, _ZEROERR_TEST_CONTEXT) \
        << [](zeroerr::TestContext * _ZEROERR_TEST_CONTEXT)

#define ZEROERR_CREATE_TEST_CLASS(fixture, classname, funcname, name)                        \
    class classname : public fixture {                                                       \
    public:                                                                                  \
        void funcname(zeroerr::TestContext*);                                                \
    };                                                                                       \
    static void ZEROERR_CAT(call_, funcname)(zeroerr::TestContext * _ZEROERR_TEST_CONTEXT) { \
        classname instance;                                                                  \
        instance.funcname(_ZEROERR_TEST_CONTEXT);                                            \
    }                                                                                        \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(                           \
        {name, __FILE__, __LINE__, ZEROERR_CAT(call_, funcname)});                           \
    inline void classname::funcname(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT)

#define TEST_CASE_FIXTURE(fixture, name)                                \
    ZEROERR_CREATE_TEST_CLASS(fixture, ZEROERR_NAMEGEN(_zeroerr_class), \
                              ZEROERR_NAMEGEN(_zeroerr_test_method), name)


namespace zeroerr {

class TestContext {
public:
    unsigned passed = 0, warning = 0, failed = 0, skipped = 0;
    unsigned passed_as = 0, warning_as = 0, failed_as = 0, skipped_as = 0;

    int add(TestContext&& local);
};

class UnitTest {
public:
    UnitTest& parseArgs(int argc, char** argv);
    int       run();
    bool      silent = false;
};

struct TestCase {
    std::string name;
    std::string file;
    unsigned    line;
    void (*func)(TestContext*);
    bool operator<(const TestCase& rhs) const;
};

struct SubCaseReg {
    SubCaseReg(std::string name, std::string file, unsigned line, TestContext* context);
    ~SubCaseReg() {}
    TestContext* context;

    void operator<<(std::function<void(TestContext*)> op);
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
