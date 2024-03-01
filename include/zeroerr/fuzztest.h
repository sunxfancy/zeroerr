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


ZEROERR_SUPPRESS_COMMON_WARNINGS_POP