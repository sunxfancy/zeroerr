#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/typetraits.h"

#include "zeroerr/domains/aggregate_of.h"
#include "zeroerr/domains/container_of.h"
#include "zeroerr/domains/domain.h"

#include <limits>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T, typename = void>
class Arbitrary {
    static_assert(detail::always_false<T>, "No Arbitrary specialization for this type");
};

template <>
class Arbitrary<bool> : public DomainConvertable<bool> {
public:
    using ValueType  = bool;
    using CorpusType = bool;

    CorpusType GetRandomCorpus(Rng& rng) const override { return rng.bounded(2); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override { v = !v; }
};

template <typename T>
class Arbitrary<
    T, typename std::enable_if<std::is_integral<T>::value && !std::numeric_limits<T>::is_signed,
                               void>::type> : public DomainConvertable<T> {
public:
    using ValueType  = T;
    using CorpusType = T;

    CorpusType GetRandomCorpus(Rng& rng) const override { return static_cast<T>(rng.bounded(100)); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = static_cast<T>(rng.bounded(100));
    }
};


template <typename T>
class Arbitrary<T, typename std::enable_if<
                       std::is_integral<T>::value && std::numeric_limits<T>::is_signed, void>::type>
    : public DomainConvertable<T> {
public:
    using ValueType  = T;
    using CorpusType = T;

    CorpusType GetRandomCorpus(Rng& rng) const override { return static_cast<T>(rng.bounded(100)); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = static_cast<T>(rng.bounded(100));
        v -= 50;
    }
};


template <typename T>
class Arbitrary<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
    : public DomainConvertable<T> {
public:
    using ValueType  = T;
    using CorpusType = T;

    CorpusType GetRandomCorpus(Rng& rng) const override {
        return static_cast<T>(rng.bounded(1000));
    }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = static_cast<T>(rng.bounded(1000));
    }
};


template <typename T>
class Arbitrary<
    T, typename std::enable_if<detail::is_specialization<T, std::basic_string>::value>::type>
    : public Arbitrary<std::vector<typename T::value_type>> {
public:
    using ValueType  = T;
    using CorpusType = std::vector<typename T::value_type>;

};


template <typename T>
class Arbitrary<
    T, typename std::enable_if<!detail::is_specialization<T, std::basic_string>::value,
                               decltype(
                                   // Iterable
                                   T().begin(), T().end(), T().size(),
                                   // Values are mutable
                                   // This rejects associative containers, for example
                                   // *T().begin() = std::declval<value_type_t<T>>(),
                                   // Can insert and erase elements
                                   T().insert(T().end(), std::declval<typename T::value_type>()),
                                   T().erase(T().begin()),
                                   //
                                   (void)0)>::type>
    : public SequenceContainerOf<T, Arbitrary<typename T::value_type>> {
public:
    Arbitrary()
        : SequenceContainerOf<T, Arbitrary<typename T::value_type>>(
              Arbitrary<typename T::value_type>{}) {}
};

template <typename T, typename U>
class Arbitrary<std::pair<T, U>>
    : public AggregateOf<std::pair<std::remove_const_t<T>, std::remove_const_t<U>>> {};


template <typename... T>
class Arbitrary<std::tuple<T...>> : public AggregateOf<std::tuple<std::remove_const_t<T>...>> {};

template <typename T>
class Arbitrary<const T> : public Arbitrary<T> {};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP