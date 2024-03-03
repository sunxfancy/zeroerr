#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/domains/domain.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T>
class Arbitrary : public Domain<T>{
public:
    using ValueType = T;
    using CorpusType = T;

};

template <>
class Arbitrary<bool> : public Domain<bool> {
public:
    using ValueType = bool;
    using CorpusType = bool;

    ValueType GetRandomValue(Rng& rng) override {
        return rng.bounded(2);
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = !v;
    }

};

template <>
class Arbitrary<int> : public Domain<int> {
public:
    using ValueType = int;
    using CorpusType = int;

    ValueType GetRandomValue(Rng& rng) override {
        return rng.bounded(100);
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = rng.bounded(100);
    }
};


template <>
class Arbitrary<std::string> : public Domain<std::string> {
public:
    using ValueType = std::string;
    using CorpusType = std::string;

    ValueType GetRandomValue(Rng& rng) override {
        return "arbitrary";
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = "arbitrary";
    }

};



} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP