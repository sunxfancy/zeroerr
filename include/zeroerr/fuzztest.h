#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/unittest.h"

#include <functional>
#include <string>
#include <vector>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

#define ZEROERR_CREATE_FUZZ_TEST_FUNC(function, name)                                     \
    static void                     function(zeroerr::TestContext*);                      \
    static zeroerr::detail::regTest ZEROERR_NAMEGEN(_zeroerr_reg)(                        \
        {name, __FILE__, __LINE__, function}, zeroerr::detail::regTest::Type::fuzz_test); \
    static void function(ZEROERR_UNUSED(zeroerr::TestContext* _ZEROERR_TEST_CONTEXT))

#define FUZZ_TEST_CASE(name) ZEROERR_CREATE_FUZZ_TEST_FUNC(ZEROERR_NAMEGEN(_zeroerr_testcase), name)

#define FUZZ_FUNC(func) zeroerr::FuzzFunction(func, _ZEROERR_TEST_CONTEXT)


namespace zeroerr {

struct FuzzTest {

    template <typename... T>
    FuzzTest& WithDomains(T&&... domains) {

        return *this;
    }

    template <typename T>
    FuzzTest& WithSeeds(std::initializer_list<T>&& seeds) {

        return *this;
    }

    void Run(int count = 1000, int seed = 0) {

    }
};

template <typename T>
FuzzTest FuzzFunction(std::function<T> func, TestContext* context) {
    
    return FuzzTest();
}

template <typename T>
std::vector<T> ReadCorpusFromDir(std::string dir);

}  // namespace zeroerr


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP