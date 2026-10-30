#pragma once

#include "zeroerr/domains/domain.h"
#include <cmath>
#include <random>
#include <stdexcept>
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

    InRange(T min, T max) : min(min), max(max) { validate(); }

    CorpusType GetRandomCorpus(Rng& rng) const override {
        validate();
        return sample(rng);
    }

    void Mutate(Rng& rng, CorpusType& v, bool) const override {
        v = GetRandomCorpus(rng);
    }

private:
    void validate() const {
        if (!(min <= max) || !std::isfinite(static_cast<long double>(min)) ||
            !std::isfinite(static_cast<long double>(max)))
            throw std::invalid_argument("InRange requires finite ordered bounds");
    }

    template <class U = T>
    typename std::enable_if<std::is_integral<U>::value, T>::type sample(Rng& rng) const {
        // Promote small integers because standard distributions exclude char and bool.
        using Int = typename std::conditional<std::is_signed<U>::value, int64_t, uint64_t>::type;
        std::uniform_int_distribution<Int> distribution(min, max);
        return static_cast<T>(distribution(rng));
    }

    template <class U = T>
    typename std::enable_if<std::is_floating_point<U>::value, T>::type sample(Rng& rng) const {
        if (min == max) return min;
        const T fraction = static_cast<T>(rng.uniform01());
        // Opposite-sign extremes may overflow max - min.
        T value = min < 0 && max > 0
                      ? min * (T(1) - fraction) + max * fraction
                      : min + (max - min) * fraction;
        return value < min ? min : value > max ? max : value;
    }
};

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP
