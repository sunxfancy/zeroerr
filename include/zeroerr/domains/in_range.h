#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/domains/domain.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T>
class InRange : public Domain<T> {
public:
    using ValueType = T;
    using CorpusType = T;

    ValueType min, max;

    InRange(T min, T max) : min(min), max(max) {}

    ValueType GetRandomValue(Rng& rng) override {
        ValueType offsize = max - min + 1;
        ValueType v = rng.bounded(offsize);
        v = min + v;
        return v;
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        CorpusType offsize = max - min + 1;
        v = rng.bounded(offsize);
        v = min + v;
    }

};  

} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP