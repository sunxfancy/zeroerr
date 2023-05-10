#pragma once

#include "zeroerr/format.h"
#include "zeroerr/internal/config.h"
#include "zeroerr/internal/debugbreak.h"
#include "zeroerr/internal/decomposition.h"
#include "zeroerr/print.h"

#include <cstdint>
#include <exception>
#include <iostream>

#pragma region define macros

// This macro will be redefined in the log.h header, so the global context variable could be
// envolved only when we use log.h at the same time. If you didn't use log.h, this is still a
// header-only library.
#ifndef ZEROERR_G_CONTEXT_SCOPE
#define ZEROERR_G_CONTEXT_SCOPE(x)
#endif

#ifndef ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER
#define ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER(cond, level, ...)                    \
    do {                                                                          \
        if (cond) {                                                               \
            switch (zeroerr::assert_level::ZEROERR_CAT(level, _l)) {              \
                case zeroerr::assert_level::ZEROERR_WARN_l:                       \
                    std::cerr << zeroerr::FgYellow << "WARN" << zeroerr::Reset;   \
                    break;                                                        \
                case zeroerr::assert_level::ZEROERR_ERROR_l:                      \
                    std::cerr << zeroerr::FgRed << "ERROR" << zeroerr::Reset;     \
                    break;                                                        \
                case zeroerr::assert_level::ZEROERR_FATAL_l:                      \
                    std::cerr << zeroerr::FgMagenta << "FATAL" << zeroerr::Reset; \
                    break;                                                        \
            }                                                                     \
            std::cerr << zeroerr::format(__VA_ARGS__) << std::endl;               \
        }                                                                         \
    } while (0)
#endif

#ifdef ZEROERR_OS_WINDOWS
#define ZEROERR_PRINT_ASSERT(cond, level, pattern, ...)                                    \
    ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER(cond, level, " Assertion Failed:\n{msg}" pattern, \
                                         assertion_data.log(), __VA_ARGS__)
#else
#define ZEROERR_PRINT_ASSERT(cond, level, pattern, ...)                                    \
    ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER(cond, level, " Assertion Failed:\n{msg}" pattern, \
                                         assertion_data.log(), ##__VA_ARGS__)
#endif

#define ZEROERR_ASSERT(cond, level, throws, is_false, ...)                                       \
    ZEROERR_FUNC_SCOPE_BEGIN {                                                                   \
        zeroerr::assert_info info{zeroerr::assert_level::ZEROERR_CAT(level, _l),                 \
                                  zeroerr::assert_throw::throws, is_false};                      \
                                                                                                 \
        zeroerr::AssertionData assertion_data(__FILE__, __LINE__, #cond, info);                  \
        assertion_data.setResult(std::move(zeroerr::ExpressionDecomposer() << cond));            \
        zeroerr::detail::context_helper<                                                         \
            decltype(_ZEROERR_TEST_CONTEXT),                                                     \
            std::is_same<decltype(_ZEROERR_TEST_CONTEXT),                                        \
                         const bool>::value>::setContext(assertion_data, _ZEROERR_TEST_CONTEXT); \
        ZEROERR_PRINT_ASSERT(assertion_data.passed == false, level, __VA_ARGS__);                \
        if (false) debug_break();                                                                \
        assertion_data();                                                                        \
        ZEROERR_FUNC_SCOPE_RET(assertion_data.passed);                                           \
    }                                                                                            \
    ZEROERR_FUNC_SCOPE_END


#define ZEROERR_ASSERT_CMP(lhs, op, rhs, level, throws, is_false, ...)                           \
    ZEROERR_FUNC_SCOPE_BEGIN {                                                                   \
        zeroerr::assert_info info{zeroerr::assert_level::ZEROERR_CAT(level, _l),                 \
                                  zeroerr::assert_throw::throws, is_false};                      \
                                                                                                 \
        zeroerr::Printer print;                                                                           \
        print.isQuoted = false;                                                                  \
        zeroerr::AssertionData assertion_data(__FILE__, __LINE__, #lhs " " #op " " #rhs, info);  \
        assertion_data.setResult({(lhs)op(rhs), print(lhs, #op, rhs)});                          \
        zeroerr::detail::context_helper<                                                         \
            decltype(_ZEROERR_TEST_CONTEXT),                                                     \
            std::is_same<decltype(_ZEROERR_TEST_CONTEXT),                                        \
                         const bool>::value>::setContext(assertion_data, _ZEROERR_TEST_CONTEXT); \
        ZEROERR_PRINT_ASSERT(assertion_data.passed == false, level, __VA_ARGS__);                \
        if (false) debug_break();                                                                \
        assertion_data();                                                                        \
        ZEROERR_FUNC_SCOPE_RET(assertion_data.passed);                                           \
    }                                                                                            \
    ZEROERR_FUNC_SCOPE_END


#ifdef ZEROERR_NO_ASSERT

#define CHECK(cond, ...)
#define CHECK_NOT(cond, ...)
#define REQUIRE(cond, ...)
#define REQUIRE_NOT(cond, ...)
#define ASSERT(cond, ...)
#define ASSERT_NOT(cond, ...)

#define CHECK_EQ(lhs, rhs, ...)
#define CHECK_NE(lhs, rhs, ...)
#define CHECK_LT(lhs, rhs, ...)
#define CHECK_LE(lhs, rhs, ...)
#define CHECK_GT(lhs, rhs, ...)
#define CHECK_GE(lhs, rhs, ...)

#define REQUIRE_EQ(lhs, rhs, ...)
#define REQUIRE_NE(lhs, rhs, ...)
#define REQUIRE_LT(lhs, rhs, ...)
#define REQUIRE_LE(lhs, rhs, ...)
#define REQUIRE_GT(lhs, rhs, ...)
#define REQUIRE_GE(lhs, rhs, ...)

#define ASSERT_EQ(lhs, rhs, ...)
#define ASSERT_NE(lhs, rhs, ...)
#define ASSERT_LT(lhs, rhs, ...)
#define ASSERT_LE(lhs, rhs, ...)
#define ASSERT_GT(lhs, rhs, ...)
#define ASSERT_GE(lhs, rhs, ...)

#else

#define CHECK(cond, ...)       ZEROERR_ASSERT(cond, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define CHECK_NOT(cond, ...)   ZEROERR_ASSERT(cond, ZEROERR_WARN, no_throw, true, __VA_ARGS__)
#define REQUIRE(cond, ...)     ZEROERR_ASSERT(cond, ZEROERR_ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_NOT(cond, ...) ZEROERR_ASSERT(cond, ZEROERR_ERROR, throws, true, __VA_ARGS__)
#define ASSERT(cond, ...)      ZEROERR_ASSERT(cond, ZEROERR_FATAL, throws, false, __VA_ARGS__)
#define ASSERT_NOT(cond, ...)  ZEROERR_ASSERT(cond, ZEROERR_FATAL, throws, true, __VA_ARGS__)

#define CHECK_EQ(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, ==, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define CHECK_NE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, !=, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define CHECK_LT(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, <, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define CHECK_LE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, <=, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define CHECK_GT(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, >, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define CHECK_GE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, >=, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)

#define REQUIRE_EQ(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, ==, rhs, ZEROERR_ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_NE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, !=, rhs, ZEROERR_ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_LT(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, <, rhs, ZEROERR_ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_LE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, <=, rhs, ZEROERR_ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_GT(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, >, rhs, ZEROERR_ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_GE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, >=, rhs, ZEROERR_ERROR, throws, false, __VA_ARGS__)

#define ASSERT_EQ(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, ==, rhs, ZEROERR_FATAL, throws, false, __VA_ARGS__)
#define ASSERT_NE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, !=, rhs, ZEROERR_FATAL, throws, false, __VA_ARGS__)
#define ASSERT_LT(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, <, rhs, ZEROERR_FATAL, throws, false, __VA_ARGS__)
#define ASSERT_LE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, <=, rhs, ZEROERR_FATAL, throws, false, __VA_ARGS__)
#define ASSERT_GT(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, >, rhs, ZEROERR_FATAL, throws, false, __VA_ARGS__)
#define ASSERT_GE(lhs, rhs, ...) \
    ZEROERR_ASSERT_CMP(lhs, >=, rhs, ZEROERR_FATAL, throws, false, __VA_ARGS__)

#endif

#pragma endregion


// This symbol must be in the global namespace or anonymous namespace
// used for checking the assert is inside testing or not
namespace {
constexpr bool _ZEROERR_TEST_CONTEXT = false;
}  // namespace


namespace zeroerr {

enum class assert_level : uint8_t { ZEROERR_WARN_l, ZEROERR_ERROR_l, ZEROERR_FATAL_l };
enum class assert_throw : uint8_t { no_throw, throws, throws_as };
enum class assert_cmp : uint8_t { eq, ne, gt, ge, lt, le };

// This is a one-byte assert info struct, which is used to collect the meta info of an assertion
struct assert_info {
    assert_level level      : 2;
    assert_throw throw_type : 2;
    bool         is_false   : 1;
};


#pragma region handle message


struct AssertionData : std::exception {
    const char* file;
    unsigned    line;
    assert_info info;
    bool        passed;
    std::string message;
    std::string cond;

    AssertionData(const char* file, unsigned line, const char* cond, assert_info info)
        : file(file), line(line), cond(cond), info(info) {}

    void setResult(ExprResult&& result) {
        ExprResult r(std::move(result));
        if (info.is_false)
            passed = !r.res;
        else
            passed = r.res;
        message = r.decomp;
    }

    std::string log() {
        std::stringstream ss;
        ss << "    " << cond << "  expands to  " << message << std::endl;
        ss << Dim << "(" << file << ":" << line << ")" << Reset << std::endl;
        return ss.str();
    }


    // throw the exception
    void operator()() {
        if (passed) return;
        if (shouldThrow()) throw *this;
    }

    bool shouldThrow() { return info.throw_type == assert_throw::throws; }
};

namespace detail {
// This struct is used for handle constexpr if in C++11
// https://stackoverflow.com/questions/43587405/constexpr-if-alternative
template <typename T, bool>
struct context_helper;

template <typename T>
struct context_helper<T, true> {
    static void setContext(AssertionData& data, T) {
        if (data.passed) return;
    }
};

template <typename T>
struct context_helper<T, false> {
    static void setContext(AssertionData& data, T ctx) {
        if (data.passed) {
            ctx->passed_as++;
            return;
        }
        switch (data.info.level) {
            case assert_level::ZEROERR_FATAL_l:
            case assert_level::ZEROERR_ERROR_l: ctx->failed_as++; break;
            case assert_level::ZEROERR_WARN_l: ctx->warning_as++; break;
        }
    }
};
}  // namespace detail

#pragma endregion


}  // namespace zeroerr