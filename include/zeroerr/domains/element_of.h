#pragma once

#include "zeroerr/domains/domain.h"
#include "zeroerr/internal/config.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

/**
 * @brief ElementOf is a domain that generates random values from a fixed set of elements
 * 
 * @tparam T The type of elements to generate
 * 
 * This domain allows generating random values by selecting from a predefined set of elements.
 * The elements are provided as a vector during construction.
 * 
 * Example:
 * ```cpp
 * // Generate random values from a set of strings
 * auto domain = ElementOf<std::string>({"red", "green", "blue"});
 * 
 * // Generate random values from a set of integers
 * auto domain = ElementOf<int>({1, 2, 3, 4, 5});
 * ```
 */
template <typename T>
class ElementOf : public Domain<T, uint64_t> {
public:
    using ValueType  = T;
    using CorpusType = uint64_t;

    std::vector<T> elements;

    ElementOf(std::vector<T> elements) : elements(elements) {}

    CorpusType GetRandomCorpus(Rng& rng) override { return rng.bounded(elements.size()); }

    ValueType GetValue(const CorpusType& v) const override { return elements[v]; }

    CorpusType FromValue(const ValueType& v) const override {
        for (size_t i = 0; i < elements.size(); i++) {
            if (elements[i] == v) return i;
        }
        return 0;
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        if (elements.size() <= 1) return;
        if (only_shrink) {
            v = rng.bounded(v);
        } else {
            v = rng.bounded(elements.size());
        }
    }
};

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP