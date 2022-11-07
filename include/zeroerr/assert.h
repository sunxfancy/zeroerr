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

#ifndef ZEROERR_PRINT_ASSERT_0
#define ZEROERR_PRINT_ASSERT_0(cond, level, ...)                             \
    do {                                                                     \
        if (cond) {                                                          \
            if (zeroerr::assert_level::level == zeroerr::assert_level::WARN) \
                std::cerr << zeroerr::FgYellow << #level << zeroerr::Reset;  \
            else                                                             \
                std::cerr << zeroerr::FgRed << #level << zeroerr::Reset;     \
            std::cerr << zeroerr::format(__VA_ARGS__) << std::endl;          \
        }                                                                    \
    } while (0)
#endif

#define ZEROERR_PRINT_ASSERT_1(cond, level, ...) \
    ZEROERR_PRINT_ASSERT_0(cond, level, " Assertion Failed:\n{msg}", assertion_data.log())

#define ZEROERR_PRINT_ASSERT(cond, level, ...) \
    ZEROERR_CAT(ZEROERR_PRINT_ASSERT_, ISEMPTY(__VA_ARGS__))(cond, level, __VA_ARGS__)


#define ZEROERR_ASSERT(cond, level, throws, is_false, ...)                                       \
    ZEROERR_FUNC_SCOPE_BEGIN {                                                                   \
        zeroerr::assert_info info{zeroerr::assert_level::level, zeroerr::assert_throw::throws,   \
                                  is_false};                                                     \
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


#ifdef ZEROERR_NO_ASSERT

#define CHECK(cond, ...)
#define CHECK_NOT(cond, ...)
#define REQUIRE(cond, ...)
#define REQUIRE_NOT(cond, ...)
#define ASSERT(cond, ...)
#define ASSERT_NOT(cond, ...)

#else

#define CHECK(cond, ...)       ZEROERR_ASSERT(cond, WARN, no_throw, false, __VA_ARGS__)
#define CHECK_NOT(cond, ...)   ZEROERR_ASSERT(cond, WARN, no_throw, true, __VA_ARGS__)
#define REQUIRE(cond, ...)     ZEROERR_ASSERT(cond, ERROR, throws, false, __VA_ARGS__)
#define REQUIRE_NOT(cond, ...) ZEROERR_ASSERT(cond, ERROR, throws, true, __VA_ARGS__)
#define ASSERT(cond, ...)      ZEROERR_ASSERT(cond, FATAL, fatal, false, __VA_ARGS__)
#define ASSERT_NOT(cond, ...)  ZEROERR_ASSERT(cond, FATAL, fatal, true, __VA_ARGS__)

#endif

#pragma endregion


// This symbol must be in the global namespace
// used for checking the assert is inside testing or not
namespace {
constexpr bool _ZEROERR_TEST_CONTEXT = false;
}  // namespace


namespace zeroerr {

enum class assert_level : uint8_t { WARN, ERROR, FATAL };
enum class assert_throw : uint8_t { fatal, no_throw, throws, throws_as };
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

    bool shouldThrow() {
        return info.throw_type == assert_throw::throws || info.throw_type == assert_throw::fatal;
    }
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
            case assert_level::FATAL:
            case assert_level::ERROR: ctx->failed_as++; break;
            case assert_level::WARN: ctx->warning_as++; break;
        }
    }
};
}  // namespace detail

#pragma endregion


}  // namespace zeroerr