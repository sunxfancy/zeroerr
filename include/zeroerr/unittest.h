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
struct TestCase;

/**
 * @brief TestContext is a class that holds the test results and reporter context.
 * There are 8 different matrices that are used to store the test results.
 * * passed    : Number of passed tests
 * * warning   : Number of tests that passed with warning
 * * failed    : Number of failed tests
 * * skipped   : Number of skipped tests
 * * passed_as : Number of passed tests in assertion
 * * warning_as: Number of tests that passed with warning in assertion
 * * failed_as : Number of failed tests in assertion
 * * skipped_as: Number of skipped tests in assertion
 */
class TestContext {
public:
    unsigned passed = 0, warning = 0, failed = 0, skipped = 0;
    unsigned passed_as = 0, warning_as = 0, failed_as = 0, skipped_as = 0;

    IReporter& reporter;

    /**
     * @brief Add the subtest results to the matrices.
     * @param local The local test context that will be added to the global context.
     * @return int  0 if the test passed, 1 if the test passed with warning, 2 if the test failed.
     */
    int add(TestContext& local);

    /**
     * @brief Reset the matrices to 0.
     */
    void reset();

    /**
     * @brief Save the output of the test to the correct_output_path as a golden file.
     */
    void save_output();

    /**
     * @brief Construct a new Test Context object
     * @param reporter The reporter object that will be used to report the test results.
     */
    TestContext(IReporter& reporter) : reporter(reporter) {}
    ~TestContext() = default;
};

/**
 * @brief UnitTest is a class that holds the test configuration.
 * There are several options that can be set to configure the test.
 * * silent          : If true, the test will not print the test results.
 * * run_bench       : If true, the test will run the benchmark tests.
 * * run_fuzz        : If true, the test will run the fuzz tests.
 * * list_test_cases : If true, the test will list the test cases.
 * * no_color        : If true, the test will not print the test results with color.
 * * log_to_report   : If true, the test will log the test results to the report.
 * * correct_output_path : The path that the golden files will be saved.
 * * reporter_name   : The name of the reporter that will be used to report the test results.
 * * binary          : The binary name that will be used to run the test.
 * * filters         : The filters that will be used to filter the test cases.
 */
struct UnitTest {
    /**
     * @brief Parse the arguments to configure the test.
     * @param argc The number of arguments.
     * @param argv The arguments.
     * @return UnitTest& The test configuration.
     */
    UnitTest& parseArgs(int argc, const char** argv);

    /**
     * @brief Run the test.
     * @return int 0 if the test passed.
     */
    int run();

    /**
     * @brief Run the test with the given filter.
     * @param tc The test case that will be run.
     * @return true If the test passed.
     * @return false If the test failed.
     */
    bool run_filter(const TestCase& tc);

    bool            silent          = false;
    bool            run_bench       = false;
    bool            run_fuzz        = false;
    bool            list_test_cases = false;
    bool            no_color        = false;
    bool            log_to_report   = false;
    std::string     correct_output_path;
    std::string     reporter_name = "console";
    std::string     binary;
    struct Filters* filters;
};

/**
 * @brief TestCase is a class that holds the test case information.
 * There are several fields that are used to store the test case information.
 * * name : The name of the test case.
 * * file : The file that the test case is defined.
 * * line : The line that the test case is defined.
 * * func : The function that will be run to test the test case.
 * * subcases : The subcases that are defined in the test case.
 */
struct TestCase {
    std::string                       name;
    std::string                       file;
    unsigned                          line;
    std::function<void(TestContext*)> func;
    std::vector<TestCase*>            subcases;

    /**
     * @brief Compare the test cases.
     * @param rhs The test case that will be compared.
     * @return true If the test case is less than the rhs, otherwise false.
     */
    bool operator<(const TestCase& rhs) const;

    /**
     * @brief Construct a new Test Case object
     * @param name The name of the test case.
     * @param file The file that the test case is defined.
     * @param line The line that the test case is defined.
     */
    TestCase(std::string name, std::string file, unsigned line)
        : name(name), file(file), line(line) {}

    /**
     * @brief Construct a new Test Case object
     * @param name The name of the test case.
     * @param file The file that the test case is defined.
     * @param line The line that the test case is defined.
     * @param func The function that will be run to test the test case.
     */
    TestCase(std::string name, std::string file, unsigned line,
             std::function<void(TestContext*)> func)
        : name(name), file(file), line(line), func(func) {}
};


/**
 * @brief SubCase is a class that holds the subcase information.
 */
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


/**
 * @brief IReporter is an interface that is used to report the test results.
 * You can create a new reporter by inheriting this class and implementing the virtual functions.
 * The following events will be called once it happens during testing.
 * * testStart     : called when the test starts.
 * * testCaseStart : called when the test case starts.
 * * testCaseEnd   : called when the test case ends.
 * * subCaseStart  : called when the subcase starts.
 * * subCaseEnd    : called when the subcase ends.
 * * testEnd       : called when the test ends.
 */
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

    /**
     * @brief Create the reporter object with the given name.
     * @param name The name of the reporter. Available reporters are: console, xml.
     * @param ut The unit test object that will be used to configure the test.
    */
    static IReporter* create(const std::string& name, UnitTest& ut);

    IReporter(UnitTest& ut) : ut(ut) {}

protected:
    UnitTest& ut;
};

/**
 * @brief TestType is a enum describe the type of the test case.
 */
enum TestType { test_case = 1, sub_case = 1 << 1, bench = 1 << 2, fuzz_test = 1 << 3 };

namespace detail {

/**
 * @brief regTest is a class that is used to register the test case.
 * It will be used as global variables and the constructor will be called to register the test case.
 */
struct regTest {
    explicit regTest(const TestCase& tc, TestType type = test_case);
};

/**
 * @brief regReporter is a class that is used to register the reporter.
 * It will be used as global variables and the constructor will be called to register the reporter.
 */
struct regReporter {
    explicit regReporter(IReporter*);
};
}  // namespace detail


/**
 * @brief CombinationalTest is a class that is used to cross test a few lists of arguments.
 * One example
 * ```cpp
 *   TestArgs<int> a{1, 2, 3};
 *   TestArgs<int> b{4, 5, 6};
 *   CombinationalTest test([&]{
 *       CHECK(targetFunc(a, b) == (a+b));
 *   });
 *   test(a, b);
 * ```
 * 
 * This will test the targetFunc with all the combinations of a and b, e.g. (1,4), (1,5), (1,6),
 * (2,4), (2,5) ... etc.
 */
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


/**
 * @brief TestArgs is a class that is used to store the test arguments.
 */
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