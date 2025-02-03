#pragma once

#include "zeroerr/domains/domain.h"
#include "zeroerr/internal/config.h"

#include <tuple>

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
#include "pfr.hpp"
#endif

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {


/**
 * @brief AggregateOf is a domain that combines multiple inner domains into a tuple or aggregate type
 * 
 * @tparam T The aggregate type to generate (e.g. struct or tuple)
 * @tparam Inner The inner domain types that will generate each field
 * 
 * This domain allows generating structured data by composing multiple inner domains.
 * Each inner domain generates one field of the aggregate type.
 * 
 * Example:
 * ```cpp
 * struct Point {
 *   int x;
 *   int y; 
 * };
 * 
 * auto domain = AggregateOf<Point>(
 *   InRange(0, 100),  // Domain for x
 *   InRange(0, 100)   // Domain for y
 * );
 * ```
 */

template <typename T, typename... Inner>
class AggregateOf : public Domain<T, std::tuple<typename Inner::CorpusType...>> {
public:
    using ValueType  = T;
    using CorpusType = std::tuple<typename Inner::CorpusType...>;

private:
    template <unsigned... I>
    inline CorpusType get_random(Rng& rng, detail::seq<I...>) const {
        return CorpusType{std::get<I>(inner_domains).GetRandomCorpus(rng)...};
    }

    template <unsigned... I>
    inline ValueType get_tuple(const CorpusType& v, detail::seq<I...>) const {
        return ValueType{std::get<I>(inner_domains).GetValue(std::get<I>(v))...};
    }

    template <unsigned... I>
    inline CorpusType from_tuple(const ValueType& v, detail::seq<I...>) const {
        return CorpusType{std::get<I>(inner_domains).FromValue(std::get<I>(v))...};
    }

    std::tuple<Inner...> inner_domains;

public:
    AggregateOf(Inner&&... inner) : inner_domains(std::make_tuple(std::move(inner)...)) {}

    CorpusType GetRandomCorpus(Rng& rng) const override {
        return get_random(rng, detail::gen_seq<sizeof...(Inner)>{});
    }

    ValueType GetValue(const CorpusType& v) const override {
        return get_tuple(v, detail::gen_seq<sizeof...(Inner)>{});
    }

    CorpusType FromValue(const ValueType& v) const override {
        return from_tuple(v, detail::gen_seq<sizeof...(Inner)>{});
    }

    struct GetTupleDomainMapValue {
        Rng& rng;
        bool only_shrink;

        template <typename D, typename H>
        void operator()(const D& domain, H& value) {
            domain.Mutate(rng, value, only_shrink);
        }
    };

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        unsigned               index = rng.bounded(sizeof...(Inner));
        GetTupleDomainMapValue visitor{rng, only_shrink};
        detail::visit2_at(inner_domains, v, index, visitor);
    }
};


#ifdef ZEROERR_ENABLE_PFR

template <typename T, typename... Inner>
AggregateOfImpl<T, Inner...> StructOf(Inner&&... inner) {
    return AggregateOfImpl<T, Inner...>(std::move(inner)...);
}

#endif

template <typename... Inner>
AggregateOf<std::tuple<typename Inner::ValueType...>, Inner...> TupleOf(Inner&&... inner) {
    return AggregateOf<std::tuple<typename Inner::ValueType...>, Inner...>(std::move(inner)...);
}

template <typename K, typename V>
AggregateOf<std::pair<typename K::ValueType, typename V::ValueType>, K, V> PairOf(K&& k, V&& v) {
    return AggregateOf<std::pair<typename K::ValueType, typename V::ValueType>, K, V>(std::move(k),
                                                                                      std::move(v));
}


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP