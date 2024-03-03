#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/domains/domain.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T>
class InRange : public Domain<T> {
    using ValueType = T;
    using CorpusType = T;

    ValueType min, max;

public:
    InRange(T min, T max) : min(min), max(max) {}

    ValueType GetRandomValue(Rng& rng) override {
        return elements[rng.bounded(elements.size())];
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        ValueType offsize = max - min + 1;
        v = rng.bounded(offsize);
        v = min + v;
    }

};  

} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP