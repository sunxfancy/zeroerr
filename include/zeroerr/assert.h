#pragma once

#include "zeroerr/internal/config.h"

#include "zeroerr/format.h"
#include "zeroerr/internal/debugbreak.h"
#include "zeroerr/internal/decomposition.h"
#include "zeroerr/print.h"

#include <cstdint>
#include <exception>
#include <iostream>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH


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
ZEROERR_CLANG_SUPPRESS_WARNING_WITH_PUSH("-Wgnu-zero-variadic-macro-arguments")
#define ZEROERR_PRINT_ASSERT(cond, level, pattern, ...)                                    \
    ZEROERR_PRINT_ASSERT_DEFAULT_PRINTER(cond, level, " Assertion Failed:\n{msg}" pattern, \
                                         assertion_data.log(), ##__VA_ARGS__)
ZEROERR_CLANG_SUPPRESS_WARNING_POP
#endif

#define ZEROERR_ASSERT_EXP(cond, level, expect_throw, is_false, ...)                             \
    ZEROERR_FUNC_SCOPE_BEGIN {                                                                   \
        zeroerr::assert_info info{zeroerr::assert_level::ZEROERR_CAT(level, _l),                 \
                                  zeroerr::assert_throw::expect_throw, is_false};                \
                                                                                                 \
        zeroerr::AssertionData assertion_data(__FILE__, __LINE__, #cond, info);                  \
        try {                                                                                    \
            assertion_data.setResult(zeroerr::ExpressionDecomposer() << cond);                   \
        } catch (const std::exception& e) {                                                      \
            assertion_data.setException(e);                                                      \
        }                                                                                        \
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


#define ZEROERR_ASSERT_CMP(lhs, op, rhs, level, expect_throw, is_false, ...)                     \
    ZEROERR_FUNC_SCOPE_BEGIN {                                                                   \
        zeroerr::assert_info info{zeroerr::assert_level::ZEROERR_CAT(level, _l),                 \
                                  zeroerr::assert_throw::expect_throw, is_false};                \
                                                                                                 \
        zeroerr::Printer print;                                                                  \
        print.isQuoted = false;                                                                  \
        zeroerr::AssertionData assertion_data(__FILE__, __LINE__, #lhs " " #op " " #rhs, info);  \
        try {                                                                                    \
            assertion_data.setResult({(lhs)op(rhs), print(lhs, #op, rhs)});                      \
        } catch (const std::exception& e) {                                                      \
            assertion_data.setException(e);                                                      \
        }                                                                                        \
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

#define CHECK(...)
#define CHECK_NOT(...)
#define CHECK_THROWS(...)
#define REQUIRE(...)
#define REQUIRE_NOT(...)
#define REQUIRE_THROWS(...)
#define ASSERT(...)
#define ASSERT_NOT(...)
#define ASSERT_THROWS(...)

#define CHECK_EQ(...)
#define CHECK_NE(...)
#define CHECK_LT(...)
#define CHECK_LE(...)
#define CHECK_GT(...)
#define CHECK_GE(...)

#define REQUIRE_EQ(...)
#define REQUIRE_NE(...)
#define REQUIRE_LT(...)
#define REQUIRE_LE(...)
#define REQUIRE_GT(...)
#define REQUIRE_GE(...)

#define ASSERT_EQ(...)
#define ASSERT_NE(...)
#define ASSERT_LT(...)
#define ASSERT_LE(...)
#define ASSERT_GT(...)
#define ASSERT_GE(...)

#else
// clang-format off
ZEROERR_CLANG_SUPPRESS_WARNING_WITH_PUSH("-Wgnu-zero-variadic-macro-arguments")

#define ZEROERR_CHECK(cond, ...)          ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_WARN, no_throw, false, __VA_ARGS__))
#define ZEROERR_CHECK_NOT(cond, ...)      ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_WARN, no_throw, true, __VA_ARGS__))
#define ZEROERR_CHECK_THROWS(cond, ...)   ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_WARN, throws, false, __VA_ARGS__))
#define ZEROERR_REQUIRE(cond, ...)        ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_ERROR, no_throw, false, __VA_ARGS__))
#define ZEROERR_REQUIRE_NOT(cond, ...)    ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_ERROR, no_throw, true, __VA_ARGS__))
#define ZEROERR_REQUIRE_THROWS(cond, ...) ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_ERROR, throws, false, __VA_ARGS__))
#define ZEROERR_ASSERT(cond, ...)         ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_FATAL, no_throw, false, __VA_ARGS__))
#define ZEROERR_ASSERT_NOT(cond, ...)     ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_FATAL, no_throw, true, __VA_ARGS__))
#define ZEROERR_ASSERT_THROWS(cond, ...)  ZEROERR_EXPAND(ZEROERR_ASSERT_EXP(cond, ZEROERR_FATAL, throws, false, __VA_ARGS__))

#define CHECK(...)          ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_NOT(...)      ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_NOT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_THROWS(...)   ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_THROWS(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE(...)        ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_NOT(...)    ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_NOT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_THROWS(...) ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_THROWS(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT(...)         ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_NOT(...)     ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_NOT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_THROWS(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_THROWS(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP

#define ZEROERR_CHECK_EQ(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, ==, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define ZEROERR_CHECK_NE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, !=, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define ZEROERR_CHECK_LT(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, <, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define ZEROERR_CHECK_LE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, <=, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define ZEROERR_CHECK_GT(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, >, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)
#define ZEROERR_CHECK_GE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, >=, rhs, ZEROERR_WARN, no_throw, false, __VA_ARGS__)

#define ZEROERR_REQUIRE_EQ(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, ==, rhs, ZEROERR_ERROR, no_throw, false, __VA_ARGS__)
#define ZEROERR_REQUIRE_NE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, !=, rhs, ZEROERR_ERROR, no_throw, false, __VA_ARGS__)
#define ZEROERR_REQUIRE_LT(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, <, rhs, ZEROERR_ERROR, no_throw, false, __VA_ARGS__)
#define ZEROERR_REQUIRE_LE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, <=, rhs, ZEROERR_ERROR, no_throw, false, __VA_ARGS__)
#define ZEROERR_REQUIRE_GT(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, >, rhs, ZEROERR_ERROR, no_throw, false, __VA_ARGS__)
#define ZEROERR_REQUIRE_GE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, >=, rhs, ZEROERR_ERROR, no_throw, false, __VA_ARGS__)

#define ZEROERR_ASSERT_EQ(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, ==, rhs, ZEROERR_FATAL, no_throw, false, __VA_ARGS__)
#define ZEROERR_ASSERT_NE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, !=, rhs, ZEROERR_FATAL, no_throw, false, __VA_ARGS__)
#define ZEROERR_ASSERT_LT(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, <, rhs, ZEROERR_FATAL, no_throw, false, __VA_ARGS__)
#define ZEROERR_ASSERT_LE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, <=, rhs, ZEROERR_FATAL, no_throw, false, __VA_ARGS__)
#define ZEROERR_ASSERT_GT(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, >, rhs, ZEROERR_FATAL, no_throw, false, __VA_ARGS__)
#define ZEROERR_ASSERT_GE(lhs, rhs, ...) ZEROERR_ASSERT_CMP(lhs, >=, rhs, ZEROERR_FATAL, no_throw, false, __VA_ARGS__)

#define CHECK_EQ(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_EQ(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_NE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_NE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_LT(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_LT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_LE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_LE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_GT(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_GT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define CHECK_GE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_CHECK_GE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_EQ(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_EQ(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_NE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_NE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_LT(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_LT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_LE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_LE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_GT(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_GT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define REQUIRE_GE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_REQUIRE_GE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_EQ(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_EQ(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_NE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_NE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_LT(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_LT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_LE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_LE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_GT(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_GT(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP
#define ASSERT_GE(...)  ZEROERR_SUPPRESS_VARIADIC_MACRO ZEROERR_EXPAND(ZEROERR_ASSERT_GE(__VA_ARGS__)) ZEROERR_SUPPRESS_VARIADIC_MACRO_POP

ZEROERR_CLANG_SUPPRESS_WARNING_POP
// clang-format on
#endif


// This symbol must be in the global namespace or anonymous namespace
// used for checking the assert is inside testing or not
namespace {
constexpr bool _ZEROERR_TEST_CONTEXT = false;
}  // namespace


namespace zeroerr {

enum class assert_level : uint8_t { ZEROERR_WARN_l, ZEROERR_ERROR_l, ZEROERR_FATAL_l };
enum class assert_throw : uint8_t { no_throw, throws, throws_as };
enum class assert_cmp : uint8_t { eq, ne, gt, ge, lt, le };

/**
 * @brief This is a one-byte assert info struct, which is used to collect the meta info of an
 * assertion
 */
struct assert_info {
    assert_level level      : 2;
    assert_throw throw_type : 2;
    bool         is_false   : 1;
};


/**
 * @brief AssertionData is a struct that contains all the information of an assertion.
 *       It will be thrown as an exception when the assertion failed.
 */
struct AssertionData : std::exception {
    const char* file;     // file name
    unsigned    line;     // line number
    assert_info info;     // assert info
    bool        passed;   // if the assertion passed
    std::string message;  // the message of the assertion
    std::string cond;     // the condition of the assertion

    AssertionData(const char* file, unsigned line, const char* cond, assert_info info)
        : file(file), line(line), info(info) {
        static std::string pattern = "zeroerr::ExpressionDecomposer() << ";
        std::string        cond_str(cond);
        size_t             pos = cond_str.find(pattern);
        if (pos != std::string::npos) cond_str.replace(pos, pos + pattern.size(), "");
        this->cond = cond_str;
    }

    void setResult(ExprResult&& result) {
        ExprResult r(std::move(result));
        if (info.is_false)
            passed = !r.res;
        else
            passed = r.res;
        message = r.decomp;
    }

    void setException(const std::exception& e) {
        passed  = info.throw_type == assert_throw::throws;
        message = e.what();
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

    bool shouldThrow() { return info.level != assert_level::ZEROERR_WARN_l; }
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
            case assert_level::ZEROERR_WARN_l:  ctx->warning_as++; break;
        }
    }
};
}  // namespace detail


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP