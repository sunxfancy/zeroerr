#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/typetraits.h"

#include "zeroerr/domains/aggregate_of.h"
#include "zeroerr/domains/container_of.h"
#include "zeroerr/domains/domain.h"

#include <limits>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T, unsigned N = 2, typename = void>
class Arbitrary : public Arbitrary<T, N-1> {};

template <typename T>
struct Arbitrary <T, 0> {
    static_assert(detail::always_false<T>::value, "No Arbitrary specialization for this type");
};

template <>
class Arbitrary<bool> : public DomainConvertable<bool> {
public:
    using ValueType  = bool;
    using CorpusType = bool;

    CorpusType GetRandomCorpus(Rng& rng) const override { return rng.bounded(2); }

    void Mutate(Rng&, CorpusType& v, bool) const override { v = !v; }
};


template <typename T>
using is_unsigned_int =
    typename std::enable_if<std::is_integral<T>::value && !std::numeric_limits<T>::is_signed,
                            void>::type;
template <typename T>
class Arbitrary<T, 2, is_unsigned_int<T>> : public DomainConvertable<T> {
public:
    using ValueType  = T;
    using CorpusType = T;

    CorpusType GetRandomCorpus(Rng& rng) const override { return static_cast<T>(rng.bounded(100)); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        v = static_cast<T>(rng.bounded(100));
    }
};

template <typename T>
using is_signed_int =
    typename std::enable_if<std::is_integral<T>::value && std::numeric_limits<T>::is_signed,
                            void>::type;

template <typename T>
class Arbitrary<T, 2, is_signed_int<T>> : public DomainConvertable<T> {
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
using is_float_point = typename std::enable_if<std::is_floating_point<T>::value, void>::type;
template <typename T>
class Arbitrary<T, 2, is_float_point<T>> : public DomainConvertable<T> {
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
using is_string =
    typename std::enable_if<detail::is_specialization<T, std::basic_string>::value>::type;

template <typename T>
class Arbitrary<T, 2, is_string<T>> : public Domain<T, std::vector<typename T::value_type>> {
    Arbitrary<std::vector<typename T::value_type>> impl;

public:
    using ValueType  = T;
    using CorpusType = std::vector<typename T::value_type>;

    ValueType GetValue(const CorpusType& v) const override { return ValueType(v.begin(), v.end()); }
    CorpusType FromValue(const ValueType& v) const override {
        return CorpusType(v.begin(), v.end());
    }

    CorpusType GetRandomCorpus(Rng& rng) const override { return impl.GetRandomCorpus(rng); }

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        impl.Mutate(rng, v, only_shrink);
    }
};

template <typename T>
using is_modifiable = typename std::enable_if<detail::is_modifiable<T>::value>::type;

template <typename T>
class Arbitrary<T, 1, is_modifiable<T>>
    : public SequenceContainerOf<T, Arbitrary<typename T::value_type>> {
public:
    Arbitrary()
        : SequenceContainerOf<T, Arbitrary<typename T::value_type>>(
              Arbitrary<typename T::value_type>{}) {}
};

template <typename T, typename U>
class Arbitrary<std::pair<T, U>, 1>
    : public AggregateOf<
          std::pair<typename std::remove_const<T>::type, typename std::remove_const<U>::type>> {};


template <typename... T>
class Arbitrary<std::tuple<T...>, 1>
    : public AggregateOf<std::tuple<typename std::remove_const<T>::type...>> {};

template <typename T>
class Arbitrary<const T, 2> : public Arbitrary<T> {};

}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP