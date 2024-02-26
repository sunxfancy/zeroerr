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


#define ZEROERR_HAVE_SAME_OUTPUT _ZEROERR_TEST_CONTEXT->save_output();


namespace zeroerr {

class TestContext {
public:
    unsigned passed = 0, warning = 0, failed = 0, skipped = 0;
    unsigned passed_as = 0, warning_as = 0, failed_as = 0, skipped_as = 0;

    int  add(TestContext&& local);
    void save_output();
};

class IReporter;
class UnitTest {
public:
    UnitTest&   parseArgs(int argc, const char** argv);
    int         run();
    bool        silent   = false;
    std::string correct_output_path;
    std::string reporter_name = "console";
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

    // There are a list of events
    virtual void testStart() = 0;
    virtual void testCaseStart(const TestCase& tc, std::stringbuf& sb) = 0;
    virtual void testCaseEnd(const TestCase& tc, std::stringbuf& sb, int type) = 0;
    virtual void testEnd(const TestContext& tc) = 0;

    static IReporter* create(const std::string& name, UnitTest& ut);

    IReporter(UnitTest& ut) : ut(ut) {}
protected:
    UnitTest& ut;
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

#pragma region combinational

class CombinationalTest {

public:
    CombinationalTest(std::function<void()> func) : func(func) {}
    std::function<void()> func;

    template <typename T>
    void operator()(T& arg) {
        arg.reset();
        for (int i = 0; i < arg.size(); ++i, ++arg) {
            func();
        }
    }

    template <typename T, typename... Args>
    void operator()(T& arg, Args&... args) {
        arg.reset();
        for (int i = 0; i < arg.size(); ++i, ++arg) {
            operator()(args...);
        }
    }
};

template<typename T>
class TestArgs {
public:
    TestArgs(std::initializer_list<T> args) : args(args) {}
    std::vector<T> args;

    operator T() const { return args[index]; }
    TestArgs& operator++() { index++; return *this; }
    int size() const { return args.size(); }
    void reset() { index = 0; }
private:
    int index = 0;
};

#pragma endregion

}  // namespace zeroerr
