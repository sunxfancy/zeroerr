#pragma once

#include "zeroerr/domains/domain.h"
#include "zeroerr/internal/config.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

/**
 * @brief InRange is a domain that generates random values within a specified range
 * 
 * @tparam T The numeric type to generate values for (e.g. int, float)
 * 
 * This domain generates random values between a minimum and maximum value (inclusive).
 * It supports any numeric type that can be used with arithmetic operations.
 * 
 * Example:
 * ```cpp
 * // Generate integers between 1 and 100
 * auto domain = InRange(1, 100);
 * 
 * // Generate floating point numbers between 0.0 and 1.0
 * auto domain = InRange(0.0, 1.0);
 * ```
 */
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