#pragma once

#include "zeroerr/config.h"

#include <set>
#include <string>


#define ZEROERR_CREATE_TEST_FUNC(f, name)                                                         \
    static void                     f();                                                          \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)({name, __FILE__, __LINE__, f}); \
    static void                     f()

#define TEST_CASE(name) ZEROERR_CREATE_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)


namespace zeroerr {

class UnitTest {
public:
    UnitTest& parseArgs(int argc, char** argv);
    int       run();
};


struct TestCase {
    std::string name;
    std::string file;
    unsigned    line;
    void (*func)();
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
