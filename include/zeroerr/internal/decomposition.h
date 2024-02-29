#pragma once

#include "zeroerr/internal/config.h"

#include "zeroerr/dbg.h"
#include "zeroerr/print.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

#ifndef ZEROERR_DISABLE_COMPLEX_AND_OR
#define AND && zeroerr::ExpressionDecomposer() <<
#define OR  || zeroerr::ExpressionDecomposer() <<
#endif

namespace zeroerr {


// SFINAE helper used to check L op R is supported, but the result type is `ret`
#define ZEROERR_SFINAE_OP(ret, op) \
    typename std::decay<decltype(std::declval<L>() op std::declval<R>(), std::declval<ret>())>::type

template <typename T>
struct deferred_false {
    static const bool value = false;
};

#define ZEROERR_EXPRESSION_COMPARISON(op, op_name)                                                 \
    template <typename R>                                                                          \
    ZEROERR_SFINAE_OP(Expression<R>, op)                                \
    operator op(R&& rhs) {                                                                         \
        std::stringstream ss;                                                                      \
        Printer           print(ss);                                                               \
        print.isCompact  = true;                                                                   \
        print.line_break = "";                                                                     \
        if (decomp.empty()) {                                                                      \
            print(lhs);                                                                            \
            res = true;                                                                            \
        } else                                                                                     \
            ss << decomp;                                                                          \
        ss << " " #op " ";                                                                         \
        print(rhs);                                                                                \
        return Expression<R>(static_cast<R&&>(rhs), res && (lhs op rhs), ss.str());                \
    }                                                                                              \
    template <typename R,                                                                          \
              typename std::enable_if<!std::is_rvalue_reference<R>::value, void>::type* = nullptr> \
    ZEROERR_SFINAE_OP(Expression<const R&>, op)                                \
    operator op(const R& rhs) {                                                                    \
        std::stringstream ss;                                                                      \
        Printer           print(ss);                                                               \
        print.isCompact  = true;                                                                   \
        print.line_break = "";                                                                     \
        if (decomp.empty()) {                                                                      \
            print(lhs);                                                                            \
            res = true;                                                                            \
        } else                                                                                     \
            ss << decomp;                                                                          \
        ss << " " #op " ";                                                                         \
        print(rhs);                                                                                \
        return Expression<const R&>(rhs, res && (lhs op rhs), ss.str());                           \
    }

#define ZEROERR_EXPRESSION_ANDOR(op, op_name)        \
    ExprResult operator op(ExprResult rhs) {         \
        std::stringstream ss;                        \
        ss << decomp << " " #op " " << rhs.decomp;   \
        return ExprResult(res op rhs.res, ss.str()); \
    }


#define ZEROERR_FORBIT_EXPRESSION(rt, op)                                 \
    template <typename R>                                                 \
    rt& operator op(const R&) {                                           \
        static_assert(deferred_false<R>::value,                           \
                      "Please Rewrite Expression As Binary Comparison!"); \
        return *this;                                                     \
    }

struct ExprResult {
    bool        res;
    std::string decomp;

    ExprResult(bool res, std::string decomposition = "") : res(res), decomp(decomposition) {}

    ZEROERR_EXPRESSION_ANDOR(&&, and)
    ZEROERR_EXPRESSION_ANDOR(||, or)

    ZEROERR_FORBIT_EXPRESSION(ExprResult, &)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, ^)
    ZEROERR_FORBIT_EXPRESSION(ExprResult, |)
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

namespace details {
    template <typename T>
    typename std::enable_if<std::is_convertible<T, bool>::value, bool>::type
    getBool(T&& lhs) {
        return static_cast<bool>(lhs);
    }

    template <typename T>
    typename std::enable_if<!std::is_convertible<T, bool>::value, bool>::type
    getBool(T&&) {
        return true;
    }
}  // namespace details

template <typename L>
struct Expression {
    L           lhs;
    bool        res = true;
    std::string decomp;

    explicit Expression(L&& in) : lhs(static_cast<L&&>(in)) { res = details::getBool(lhs); }
    explicit Expression(L&& in, bool res, std::string&& decomp)
        : lhs(static_cast<L&&>(in)), res(res), decomp(static_cast<std::string&&>(decomp)) {}
    
    operator ExprResult() {
        if (decomp.empty()) {
            Printer print;
            print.isCompact  = true;
            print.line_break = "";
            decomp           = print(lhs).str();
        }
        return ExprResult(res, decomp);
    }

    operator L() const { return lhs; }

    ZEROERR_EXPRESSION_COMPARISON(==, eq)
    ZEROERR_EXPRESSION_COMPARISON(!=, ne)
    ZEROERR_EXPRESSION_COMPARISON(>, gt)
    ZEROERR_EXPRESSION_COMPARISON(<, lt)
    ZEROERR_EXPRESSION_COMPARISON(>=, ge)
    ZEROERR_EXPRESSION_COMPARISON(<=, le)

    ZEROERR_EXPRESSION_ANDOR(&&, and)
    ZEROERR_EXPRESSION_ANDOR(||, or)

    ZEROERR_FORBIT_EXPRESSION(Expression, &)
    ZEROERR_FORBIT_EXPRESSION(Expression, ^)
    ZEROERR_FORBIT_EXPRESSION(Expression, |)
    ZEROERR_FORBIT_EXPRESSION(Expression, =)
    ZEROERR_FORBIT_EXPRESSION(Expression, +=)
    ZEROERR_FORBIT_EXPRESSION(Expression, -=)
    ZEROERR_FORBIT_EXPRESSION(Expression, *=)
    ZEROERR_FORBIT_EXPRESSION(Expression, /=)
    ZEROERR_FORBIT_EXPRESSION(Expression, %=)
    ZEROERR_FORBIT_EXPRESSION(Expression, <<=)
    ZEROERR_FORBIT_EXPRESSION(Expression, >>=)
    ZEROERR_FORBIT_EXPRESSION(Expression, &=)
    ZEROERR_FORBIT_EXPRESSION(Expression, ^=)
    ZEROERR_FORBIT_EXPRESSION(Expression, |=)
    ZEROERR_FORBIT_EXPRESSION(Expression, <<)
    ZEROERR_FORBIT_EXPRESSION(Expression, >>)
};

struct ExpressionDecomposer {
    // The right operator for capturing expressions is "<=" instead of "<<" (based on the
    // operator precedence table) but then there will be warnings from GCC about "-Wparentheses"
    // and since "_Pragma()" is problematic this will stay for now...
    // https://github.com/catchorg/Catch2/issues/870
    // https://github.com/catchorg/Catch2/issues/565

    // For temporary objects, we need to use rvalue reference to avoid copy
    template <typename L>
    Expression<L> operator<<(L&& operand) {
        return Expression<L>(static_cast<L&&>(operand));
    }

    // For other objects, we will store the reference
    template <typename L,
              typename std::enable_if<!std::is_rvalue_reference<L>::value, void>::type* = nullptr>
    Expression<const L&> operator<<(const L& operand) {
        return Expression<const L&>(operand);
    }
};


template <typename T>
class IMatcher {
public:
    virtual ~IMatcher() = default;


    virtual bool match(const T&) const = 0;
};

template <typename T>
class IMatcherRef {
public:
    IMatcherRef(const IMatcher<T>* ptr) : p(ptr) {}
    IMatcherRef(const IMatcherRef&) = delete;

    IMatcherRef(IMatcherRef&& other) {
        p       = std::move(other.p);
        other.p = nullptr;
    }
    void operator=(IMatcherRef&& other) {
        p       = std::move(other.p);
        other.p = nullptr;
    }
    IMatcherRef& operator=(const IMatcherRef&) = delete;
    ~IMatcherRef() {
        if (p) delete p;
    }


    IMatcherRef operator&&(IMatcherRef&& other);
    IMatcherRef operator||(IMatcherRef&& other);
    IMatcherRef operator!();

    const IMatcher<T>* operator->() const { return p; }

protected:
    const IMatcher<T>* p = nullptr;
};


template <typename T>
class CombinedMatcher : public IMatcher<T> {
public:
    CombinedMatcher(IMatcherRef<T>&& lhs, IMatcherRef<T>&& rhs, bool is_and)
        : lhs(std::move(lhs)), rhs(std::move(rhs)), is_and(is_and) {}

    IMatcherRef<T> lhs;
    IMatcherRef<T> rhs;
    bool           is_and;

    virtual bool match(const T& t) const override {
        if (is_and) {
            return lhs->match(t) && rhs->match(t);
        } else {
            return lhs->match(t) || rhs->match(t);
        }
    }
};

template <typename T>
class NotMatcher : public IMatcher<T> {
public:
    NotMatcher(IMatcherRef<T>&& matcher) : matcher(std::move(matcher)) {}
    IMatcherRef<T> matcher;

    virtual bool match(const T& t) const override { return !matcher->match(t); }
};

template <typename T>
inline IMatcherRef<T> IMatcherRef<T>::operator&&(IMatcherRef<T>&& other) {
    return new CombinedMatcher<T>(std::move(*this), std::move(other), true);
}

template <typename T>
inline IMatcherRef<T> IMatcherRef<T>::operator||(IMatcherRef<T>&& other) {
    return new CombinedMatcher<T>(std::move(*this), std::move(other), false);
}

template <typename T>
inline IMatcherRef<T> IMatcherRef<T>::operator!() {
    return new NotMatcher<T>(std::move(*this));
}


template <typename T>
struct StartWithMatcher : public IMatcher<T> {
    StartWithMatcher(const T& s) : start(s) {}
    T start;

    virtual bool match(const T& t) const override {
        bool result = true;
        for (auto i = start.begin(), j = t.begin(); i != start.end(); ++i, ++j) {
            if (j == t.end() || *i != *j) {
                result = false;
                break;
            }
        }
        return result;
    }
};

template <typename T>
typename std::enable_if<std::is_constructible<std::string, T>::value,
                        IMatcherRef<std::string>>::type
start_with(T&& s) {
    return new StartWithMatcher<std::string>(std::string(s));
}

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP