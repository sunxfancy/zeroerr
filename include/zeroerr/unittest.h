#pragma once

#include "zeroerr/internal/config.h"

#include <functional>
#include <string>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

#define ZEROERR_CREATE_TEST_FUNC(function, name)                     \
    static void                     function(zeroerr::TestContext*); \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(   \
        {name, __FILE__, __LINE__, function});                       \
    static void function(ZEROERR_UNUSED(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT))

#define TEST_CASE(name) ZEROERR_CREATE_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)

#define SUB_CASE(name)                                                \
    zeroerr::SubCase(name, __FILE__, __LINE__, _ZEROERR_TEST_CONTEXT) \
        << [=](ZEROERR_UNUSED(zeroerr::TestContext * _ZEROERR_TEST_CONTEXT)) mutable

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
    inline void classname::funcname(ZEROERR_UNUSED(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT))

#define TEST_CASE_FIXTURE(fixture, name)                                \
    ZEROERR_CREATE_TEST_CLASS(fixture, ZEROERR_NAMEGEN(_zeroerr_class), \
                              ZEROERR_NAMEGEN(_zeroerr_test_method), name)


#define ZEROERR_HAVE_SAME_OUTPUT _ZEROERR_TEST_CONTEXT->save_output();

#ifndef ZEROERR_DISABLE_BDD
#define SCENARIO(...) TEST_CASE("Scenario: " __VA_ARGS__)
#define GIVEN(...)    SUB_CASE("given: " __VA_ARGS__)
#define WHEN(...)     SUB_CASE("when: " __VA_ARGS__)
#define THEN(...)     SUB_CASE("then: " __VA_ARGS__)
#endif

namespace zeroerr {

class IReporter;
class TestContext {
public:
    unsigned passed = 0, warning = 0, failed = 0, skipped = 0;
    unsigned passed_as = 0, warning_as = 0, failed_as = 0, skipped_as = 0;

    IReporter& reporter;
    int        add(TestContext& local);
    void       reset();
    void       save_output();

    TestContext(IReporter& reporter) : reporter(reporter) {}
    ~TestContext() = default;
};

struct TestCase;
struct UnitTest {
    UnitTest&        parseArgs(int argc, const char** argv);
    int              run();
    bool             run_filiter(const TestCase& tc);
    bool             silent          = false;
    bool             run_bench       = false;
    bool             run_fuzz        = false;
    bool             list_test_cases = false;
    bool             no_color        = false;
    std::string      correct_output_path;
    std::string      reporter_name = "console";
    std::string      binary;
    struct Filiters* filiters;
};

struct TestCase {
    std::string                       name;
    std::string                       file;
    unsigned                          line;
    std::function<void(TestContext*)> func;
    bool                              operator<(const TestCase& rhs) const;

    std::vector<TestCase*> subcases;

    TestCase(std::string name, std::string file, unsigned line)
        : name(name), file(file), line(line) {}
    TestCase(std::string name, std::string file, unsigned line,
             std::function<void(TestContext*)> func)
        : name(name), file(file), line(line), func(func) {}
};

struct SubCase : TestCase {
    SubCase(std::string name, std::string file, unsigned line, TestContext* context);
    ~SubCase() = default;
    TestContext* context;
    void         operator<<(std::function<void(TestContext*)> op);
};


template <typename T>
struct TestedObjects {
    void           add(T&& obj) { objects.push_back(std::forward<T>(obj)); }
    std::vector<T> objects;
};


class IReporter {
public:
    virtual ~IReporter() = default;

    virtual std::string getName() const = 0;

    // There are a list of events
    virtual void testStart()                                           = 0;
    virtual void testCaseStart(const TestCase& tc, std::stringbuf& sb) = 0;
    virtual void testCaseEnd(const TestCase& tc, std::stringbuf& sb, const TestContext& ctx,
                             int type)                                 = 0;
    virtual void subCaseStart(const TestCase& tc, std::stringbuf& sb)  = 0;
    virtual void subCaseEnd(const TestCase& tc, std::stringbuf& sb, const TestContext& ctx,
                            int type)                                  = 0;
    virtual void testEnd(const TestContext& tc)                        = 0;

    static IReporter* create(const std::string& name, UnitTest& ut);

    IReporter(UnitTest& ut) : ut(ut) {}

protected:
    UnitTest& ut;
};


enum TestType { test_case = 1, sub_case = 1 << 1, bench = 1 << 2, fuzz_test = 1 << 3 };

namespace detail {
struct regTest {
    explicit regTest(const TestCase& tc, TestType type = test_case);
};

struct regReporter {
    explicit regReporter(IReporter*);
};
}  // namespace detail


class CombinationalTest {
public:
    CombinationalTest(std::function<void()> func) : func(func) {}
    std::function<void()> func;

    template <typename T>
    void operator()(T& arg) {
        arg.reset();
        for (size_t i = 0; i < arg.size(); ++i, ++arg) {
            func();
        }
    }

    template <typename T, typename... Args>
    void operator()(T& arg, Args&... args) {
        arg.reset();
        for (size_t i = 0; i < arg.size(); ++i, ++arg) {
            operator()(args...);
        }
    }
};

template <typename T>
class TestArgs {
public:
    TestArgs(std::initializer_list<T> args) : args(args) {}
    std::vector<T> args;

    operator T() const { return args[index]; }
    TestArgs& operator++() {
        index++;
        return *this;
    }
    size_t size() const { return args.size(); }
    void   reset() { index = 0; }

private:
    int index = 0;
};

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP