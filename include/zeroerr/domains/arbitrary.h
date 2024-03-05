#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/domains/aggregate_of.h"
#include "zeroerr/domains/domain.h"

#include <limits>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T, typename = void>
class Arbitrary : public Domain<T> {
public:
    using ValueType  = T;
    using CorpusType = T;
};

template <>
class Arbitrary<bool> : public Domain<bool> {
public:
    using ValueType  = bool;
    using CorpusType = bool;

    ValueType GetRandomValue(Rng& rng) override { return rng.bounded(2); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override { v = !v; }
};

template <typename T>
class Arbitrary<
    T, typename std::enable_if<std::is_integral<T>::value && !std::numeric_limits<T>::is_signed,
                               void>::type> : public Domain<T> {
public:
    using ValueType  = int;
    using CorpusType = int;

    ValueType GetRandomValue(Rng& rng) override { return rng.bounded(100); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override { v = rng.bounded(100); }
};


template <typename T>
class Arbitrary<T, typename std::enable_if<
                       std::is_integral<T>::value && std::numeric_limits<T>::is_signed, void>::type>
    : public Domain<T> {
public:
    using ValueType  = int;
    using CorpusType = int;

    ValueType GetRandomValue(Rng& rng) override { return rng.bounded(100); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = rng.bounded(100);
        v -= 50;
    }
};


template <typename T>
class Arbitrary<T, typename std::enable_if<std::is_floating_point<T>::value, void>::type>
    : public Domain<T> {
public:
    using ValueType  = int;
    using CorpusType = int;

    ValueType GetRandomValue(Rng& rng) override { return rng.bounded(1000); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override { v = rng.bounded(1000); }
};


template <>
class Arbitrary<std::string> : public Domain<std::string> {
public:
    using ValueType  = std::string;
    using CorpusType = std::string;

    ValueType GetRandomValue(Rng& rng) override { 


    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override { v = "arbitrary"; }
};

template <typename T, typename U>
class Arbitrary<std::pair<T, U>>
    : public AggregateOf<std::pair<std::remove_const_t<T>, std::remove_const_t<U>>> {};


template <typename... T>
class Arbitrary<std::tuple<T...>> : public AggregateOf<std::tuple<std::remove_const_t<T>...>> {};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP