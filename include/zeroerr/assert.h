#pragma once

#include <cstdint>
#include <exception>
#include <iostream>

#include "zeroerr/config.h"
#include "zeroerr/debugbreak.h"
#include "zeroerr/print.h"

#pragma region define macros


#define ZEROERR_ASSERT(cond, level, throws, is_false, ...)                                     \
    do {                                                                                       \
        zeroerr::assert_info info{zeroerr::assert_level::level, zeroerr::assert_throw::throws, \
                                  is_false};                                                   \
                                                                                               \
        zeroerr::AssertionData data(__FILE__, __LINE__, #cond);                                \
        data.setResult(std::move(zeroerr::ExpressionDecomposer(info) << cond));                \
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

#define ZEROERR_EXPRESSION_COMPARISON(op, op_name)    \
    template <typename R>                             \
    ZEROERR_SFINAE_OP(Result, op)                     \
    operator op(R&& rhs) {                            \
        bool res = (lhs op rhs);                      \
        info.cmp = assert_cmp::op_name;               \
        if (info.is_false) res = !res;                \
        std::stringstream ss;                         \
        Printer           print(ss);                  \
        print.isCompact  = true;                      \
        print.line_break = "";                        \
        print(lhs);                                   \
        ss << #op " ";                                \
        print(rhs);                                   \
        if (!res) return Result(res, info, ss.str()); \
        return Result(res, info);                     \
    }

#define ZEROERR_FORBIT_EXPRESSION(rt, op)                                 \
    template <typename R>                                                 \
    rt& operator op(const R&) {                                           \
        static_assert(deferred_false<R>::value,                           \
                      "Please Rewrite Expression As Binary Comparison!"); \
        return *this;                                                     \
    }

struct Result {
    bool        passed;
    std::string decomp;
    assert_info info;

    Result(bool passed, assert_info info, std::string decomposition = "")
        : passed(passed), info(info), decomp(decomposition) {}

    ZEROERR_FORBIT_EXPRESSION(Result, &)
    ZEROERR_FORBIT_EXPRESSION(Result, ^)
    ZEROERR_FORBIT_EXPRESSION(Result, |)
    ZEROERR_FORBIT_EXPRESSION(Result, &&)
    ZEROERR_FORBIT_EXPRESSION(Result, ||)
    ZEROERR_FORBIT_EXPRESSION(Result, ==)
    ZEROERR_FORBIT_EXPRESSION(Result, !=)
    ZEROERR_FORBIT_EXPRESSION(Result, <)
    ZEROERR_FORBIT_EXPRESSION(Result, >)
    ZEROERR_FORBIT_EXPRESSION(Result, <=)
    ZEROERR_FORBIT_EXPRESSION(Result, >=)
    ZEROERR_FORBIT_EXPRESSION(Result, +=)
    ZEROERR_FORBIT_EXPRESSION(Result, -=)
    ZEROERR_FORBIT_EXPRESSION(Result, *=)
    ZEROERR_FORBIT_EXPRESSION(Result, /=)
    ZEROERR_FORBIT_EXPRESSION(Result, %=)
    ZEROERR_FORBIT_EXPRESSION(Result, <<=)
    ZEROERR_FORBIT_EXPRESSION(Result, >>=)
    ZEROERR_FORBIT_EXPRESSION(Result, &=)
    ZEROERR_FORBIT_EXPRESSION(Result, ^=)
    ZEROERR_FORBIT_EXPRESSION(Result, |=)
};

template <typename L>
struct Expression_lhs {
    L           lhs;
    assert_info info;

    explicit Expression_lhs(L&& in, assert_info info) : lhs(std::forward<L>(in)), info(info) {}

    operator Result() {
        bool res = static_cast<bool>(lhs);
        if (info.is_false) res = !res;
        std::stringstream ss;

        Printer print(ss);
        print(lhs);
        if (!res) return Result(res, info, ss.str());
        return Result(res, info);
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

    void setResult(Result&& result) {
        Result r(std::move(result));
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

#pragma endregion


}  // namespace zeroerr