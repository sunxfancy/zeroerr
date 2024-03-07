#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/domains/domain.h"
#include "zeroerr/internal/typetraits.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

struct ContainerOfBase {
    int min_size = 0, max_size = 100, size = -1;

    void WithMaxSize(unsigned _max_size) { this->max_size = _max_size; }
    void WithMinSize(unsigned _min_size) { this->min_size = _min_size; }
    void WithSize(unsigned _size) { this->size = _size; }
};


template <typename T, typename InnerDomain>
class AssociativeContainerOf : public Domain<T, std::vector<typename InnerDomain::CorpusType>>,
                               public ContainerOfBase {
    InnerDomain inner_domain;

public:
    using ValueType  = T;
    using CorpusType = std::vector<typename InnerDomain::CorpusType>;

    AssociativeContainerOf(InnerDomain&& inner_domain) : inner_domain(std::move(inner_domain)) {}

    virtual ValueType GetValue(const CorpusType& v) const override {
        ValueType result;
        for (const auto& elem : v) {
            result.insert(inner_domain.GetValue(elem));
        }
        return result;
    }

    virtual CorpusType FromValue(const ValueType& v) const override {
        CorpusType result;
        for (const auto& elem : v) {
            result.push_back(inner_domain.FromValue(elem));
        }
        return result;
    }

    CorpusType GetRandomCorpus(Rng& rng) const override {
        unsigned   E = rng.bounded(max_size - min_size + 1) + min_size;
        CorpusType result;
        for (unsigned i = 0; i < E; i++) {
            result.push_back(inner_domain.GetRandomCorpus(rng));
        }
        return result;
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        int action = rng.bounded(5);
        if (size == -1 && action == 0) {
            if (static_cast<int>(v.size()) > min_size)
                v.erase(v.begin() + rng.bounded(static_cast<uint32_t>(v.size())));

        } else if (size == -1 && action == 1) {
            if (static_cast<int>(v.size()) < max_size)
                v.push_back(inner_domain.GetRandomCorpus(rng));

        } else {
            inner_domain.Mutate(rng, v[rng.bounded(static_cast<uint32_t>(v.size()))], only_shrink);
        }
    }
};

template <typename T, typename InnerDomain>
class SequenceContainerOf : public Domain<T, std::vector<typename InnerDomain::CorpusType>>,
                            public ContainerOfBase {
    InnerDomain inner_domain;

public:
    using ValueType  = T;
    using CorpusType = std::vector<typename InnerDomain::CorpusType>;

    SequenceContainerOf(InnerDomain&& inner_domain) : inner_domain(std::move(inner_domain)) {}

    virtual ValueType GetValue(const CorpusType& v) const override {
        return ValueType(v.begin(), v.end());
    }
    virtual CorpusType FromValue(const ValueType& v) const override {
        return CorpusType(v.begin(), v.end());
    }

    CorpusType GetRandomCorpus(Rng& rng) const override {
        unsigned   E = rng.bounded(max_size - min_size + 1) + min_size;
        CorpusType result;
        for (unsigned i = 0; i < E; i++) {
            result.push_back(inner_domain.GetRandomCorpus(rng));
        }
        return result;
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        int action = rng.bounded(5);
        if (size == -1 && action == 0) {
            if (static_cast<int>(v.size()) > min_size) v.erase(v.begin() + rng.bounded(static_cast<uint32_t>(v.size())));

        } else if (size == -1 && action == 1) {
            if (static_cast<int>(v.size()) < max_size)
                v.push_back(inner_domain.GetRandomCorpus(rng));
        } else {
            inner_domain.Mutate(rng, v[rng.bounded(static_cast<uint32_t>(v.size()))], only_shrink);
        }
    }
};


// ContainerOf<T>(inner) combinator creates a domain for a container T (eg, a
// std::vector, std::set, etc) where elements are created from `inner`.
//
// Example usage:
//
//   ContainerOf<std::vector<int>>(InRange(1, 2021))
//
// The domain also supports customizing the minimum and maximum size via the
// `WithSize`, `WithMinSize` and `WithMaxSize` functions. Eg:
//
//   ContainerOf<std::vector<int>>(Arbitrary<int>()).WithMaxSize(5)
//
template <typename T, typename Inner>
typename std::enable_if<detail::is_associative_container<T>::value,
                        AssociativeContainerOf<T, Inner>>::type
ContainerOf(Inner&& inner) {
    return AssociativeContainerOf<T, Inner>(std::move(inner));
}

template <typename T, typename Inner>
typename std::enable_if<!detail::is_associative_container<T>::value &&
                            detail::is_container<T>::value,
                        SequenceContainerOf<T, Inner>>::type
ContainerOf(Inner&& inner) {
    return SequenceContainerOf<T, Inner>(std::move(inner));
}

// We can also support with a template template parameter, so that we can use
// the name of the container instead of the complete type. In such case,
// the ValueType of the inner domain should be passed into the container
//
//   ContainerOf<std::vector>(Positive<int>()).WithSize(3);
//
template <template <typename, typename...> class T, typename... Inner,
          typename C = T<typename Inner::ValueType...>>
auto ContainerOf(Inner... inner) -> decltype(ContainerOf<C>(std::move(inner)...)) {
    return ContainerOf<C>(std::move(inner)...);
}


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP