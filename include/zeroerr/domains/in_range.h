#pragma once

#include "zeroerr/domains/domain.h"
#include "zeroerr/internal/config.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T>
class InRange : public DomainConvertable<T> {
public:
    using ValueType  = T;
    using CorpusType = T;

    ValueType min, max;

    InRange(T min, T max) : min(min), max(max) {}

    CorpusType GetRandomCorpus(Rng& rng) const override {
        ValueType offsize = max - min + 1;
        ValueType v       = rng.bounded(offsize);
        v                 = min + v;
        return v;
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        CorpusType offsize = max - min + 1;
        v                  = rng.bounded(offsize);
        v                  = min + v;
    }
};

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP