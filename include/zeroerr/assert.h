#pragma once

#include "zeroerr/config.h"
#include "zeroerr/debugbreak.h"
#include "zeroerr/print.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <sstream>

#pragma region define macros

// This macro will be redefined in the log.h header, so the global context variable could be
// envolved only when we use log.h at the same time. If you didn't use log.h, this is still a
// header-only library.
#ifndef ZEROERR_G_CONTEXT_SCOPE
#define ZEROERR_G_CONTEXT_SCOPE(x)
#endif

#define ZEROERR_ASSERT(cond, level, throws, is_false, ...)                                     \
    do {                                                                                       \
        zeroerr::assert_info info{zeroerr::assert_level::level, zeroerr::assert_throw::throws, \
                                  is_false};                                                   \
                                                                                               \
        zeroerr::AssertionData data(__FILE__, __LINE__, #cond);                                \
        data.ExprsetResult(std::move(zeroerr::ExpressionDecomposer(info) << cond));            \
        zeroerr::detail::context_helper<                                                       \
            decltype(_ZEROERR_TEST_CONTEXT),                                                   \
            std::is_same<decltype(_ZEROERR_TEST_CONTEXT),                                      \
                         const bool>::value>::setContext(data, _ZEROERR_TEST_CONTEXT);         \
        ZEROERR_G_CONTEXT_SCOPE(data);                                                         \
        if (data.log()) debug_break();                                                         \
        data();                                                                                \
    } while (0)


#ifdef ZEROERR_NO_ASSERT

#define WARN(cond, ...)
#define WARN_NOT(cond, ...)
#define CHECK(cond, ...)
#define CHECK_NOT(cond, ...)
#define REQUIRE(cond, ...)
#define REQUIRE_NOT(cond, ...)

#else

#define WARN(cond, ...)        ZEROERR_ASSERT(cond, warning, no_throw, false, __VA_ARGS__)
#define WARN_NOT(cond, ...)    ZEROERR_ASSERT(cond, warning, no_throw, true, __VA_ARGS__)
#define CHECK(cond, ...)       ZEROERR_ASSERT(cond, check, no_throw, false, __VA_ARGS__)
#define CHECK_NOT(cond, ...)   ZEROERR_ASSERT(cond, check, no_throw, true, __VA_ARGS__)
#define REQUIRE(cond, ...)     ZEROERR_ASSERT(cond, require, throws, false, __VA_ARGS__)
#define REQUIRE_NOT(cond, ...) ZEROERR_ASSERT(cond, require, throws, true, __VA_ARGS__)

#endif

#pragma endregion


// This symbol must be in the global namespace
// used for checking the assert is inside testing or not
namespace {
constexpr bool _ZEROERR_TEST_CONTEXT = false;
}  // namespace


namespace zeroerr {

enum class assert_level : uint8_t { warn, check, require };
enum class assert_throw : uint8_t { no_throw, throws, throws_as };
enum class assert_cmp : uint8_t { eq, ne, gt, ge, lt, le };

// This is a one-byte assert info struct, which is used to collect the meta info of an assertion
struct assert_info {
    assert_level level      : 2;
    assert_throw throw_type : 2;
    bool         is_false   : 1;
    assert_cmp   cmp        : 3;
};


#pragma region expression decompostion

// SFINAE helper used to check L op R is supported, but the result type is `ret`
#define ZEROERR_SFINAE_OP(ret, op) \
    typename std::decay<decltype(std::declval<L>() op std::declval<R>(), std::declval<ret>())>::type

template <typename T>
struct deferred_false {
    static const bool value = false;
};

#define ZEROERR_EXPRESSION_COMPARISON(op, op_name)        \
    template <typename R>                                 \
    ZEROERR_SFINAE_OP(ExprResult, op)                     \
    operator op(R&& rhs) {                                \
        bool res = (lhs op rhs);                          \
        info.cmp = assert_cmp::op_name;                   \
        if (info.is_false) res = !res;                    \
        std::stringstream ss;                             \
        Printer           print(ss);                      \
        print.isCompact  = true;                          \
        print.line_break = "";                            \
        print(lhs);                                       \
        ss << #op " ";                                    \
        print(rhs);                                       \
        if (!res) return ExprResult(res, info, ss.str()); \
        return ExprResult(res, info);                     \
    }

#define ZEROERR_FORBIT_EXPRESSION(rt, op)                                 \
    template <typename R>                                                 \
    rt& operator op(const R&) {                                           \
        static_assert(deferred_false<R>::value,                           \
                      "Please Rewrite Expression As Binary Comparison!"); \
        return *this;                                                     \
    }

struct ExprResult {
    bool        passed;
    std::string decomp;
    assert_info info;

    ExprResult(bool passed, assert_info info, std::string decomposition = "")
        : passed(passed), info(info), decomp(decomposition) {}

    ZEROERR_FORBIT_EXPRESSION(ExprResult, &)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, ^)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, |)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, &&)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, ||)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, ==)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, !=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, <)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, >)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, <=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, >=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, +=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, -=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, *=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, /=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, %=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, <<=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, >>=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, &=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, ^=)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, |=)
};

template <typename L>
struct Expression_lhs {
    L           lhs;
    assert_info info;

    explicit Expression_lhs(L&& in, assert_info info) : lhs(std::forward<L>(in)), info(info) {}

    operator ExprResult() {
        bool res = static_cast<bool>(lhs);
        if (info.is_false) res = !res;
        std::stringstream ss;

        Printer print(ss);
        print(lhs);
        if (!res) return ExprResult(res, info, ss.str());
        return ExprResult(res, info);
    }

    operator L() const { return lhs; }

    ZEROERR_EXPRESSION_COMPARISON(==, eq)
    ZEROERR_EXPRESSION_COMPARISON(!=, ne)
    ZEROERR_EXPRESSION_COMPARISON(>, gt)
    ZEROERR_EXPRESSION_COMPARISON(<, lt)
    ZEROERR_EXPRESSION_COMPARISON(>=, ge)
    ZEROERR_EXPRESSION_COMPARISON(<=, le)

    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, &)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, ^)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, |)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, &&)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, ||)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, =)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, +=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, -=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, *=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, /=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, %=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, <<=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, >>=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, &=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, ^=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, |=)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, <<)
    ZEROERR_FORBIT_EXPRESSION(Expression_lhs, >>)
};

struct ExpressionDecomposer {
    assert_info info;

    ExpressionDecomposer(assert_info info) : info(info) {}

    // The right operator for capturing expressions is "<=" instead of "<<" (based on the
    // operator precedence table) but then there will be warnings from GCC about "-Wparentheses"
    // and since "_Pragma()" is problematic this will stay for now...
    // https://github.com/catchorg/Catch2/issues/870
    // https://github.com/catchorg/Catch2/issues/565
    template <typename L>
    Expression_lhs<L> operator<<(L&& operand) {
        return Expression_lhs<L>(std::forward<L>(operand), info);
    }
};

#pragma endregion


#pragma region handle message


struct AssertionData : std::exception {
    const char* file;
    unsigned    line;
    assert_info info;
    bool        passed;
    std::string message;
    std::string cond;

    AssertionData(const char* file, unsigned line, const char* cond)
        : file(file), line(line), cond(cond) {}

    void ExprsetResult(ExprResult&& result) {
        ExprResult r(std::move(result));
        passed  = r.passed;
        info    = r.info;
        message = r.decomp;
    }

    bool log() {
        if (passed) return false;

        if (info.level == assert_level::require) {
            std::cerr << "REQUIRE: ";
        } else {
            std::cerr << "CHECK: ";
        }

        std::cerr << "Assertion Failed " << cond << ": " << message << " (" << file << ":" << line
                  << ")" << std::endl;
        return false;
    }


    // throw the exception
    void operator()() {
        if (passed) return;
        if (shouldThrow()) throw *this;
    }

    bool shouldThrow() { return info.level == assert_level::require; }
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
            case assert_level::require:
            case assert_level::check: ctx->failed_as++;
            case assert_level::warn: ctx->warning_as++;
        }
    }
};
}  // namespace detail

#pragma endregion


#pragma region matcher

class CombinedMatcher;

class IMatcher {
public:
    virtual ~IMatcher() = default;
    // virtual operator bool() = 0;

    std::string toString();

    IMatcher&& operator&&(IMatcher&& other) const;
    IMatcher&& operator||(const IMatcher&& other) const;
    IMatcher&& operator!() const;

protected:
    // virtual std::string describe() const = 0;
};

class CombinedMatcher : public IMatcher {
public:
    CombinedMatcher(const IMatcher&& lhs, const IMatcher&& rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    const IMatcher&& lhs;
    const IMatcher&& rhs;
};

class NotMatcher : public IMatcher {
public:
    NotMatcher(IMatcher& matcher) : matcher(std::move(matcher)) {}
    const IMatcher&& matcher;
};


inline IMatcher&& IMatcher::operator&&(IMatcher&& other) const {
    return std::move(CombinedMatcher(std::move(*this), std::move(other)));
}

template <typename T>
struct Matcher {};


#pragma endregion


}  // namespace zeroerr