#pragma once

#include "zeroerr/domains/domain.h"
#include "zeroerr/internal/config.h"

#include <tuple>

#if defined(ZEROERR_ENABLE_PFR) && (ZEROERR_CXX_STANDARD >= 14)
#include "pfr.hpp"
#endif

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T, typename... Inner>
class AggregateOf : public Domain<T, std::tuple<typename Inner::CorpusType...>> {
public:
    using ValueType  = T;
    using CorpusType = std::tuple<typename Inner::CorpusType...>;

private:
    // template <>
    // inline ValueType get_random(Rng& rng, detail::seq<>) {}

    template <unsigned... I>
    inline ValueType get_random(Rng& rng, detail::seq<I...>) {
        return ValueType{std::get<I>(inner_domains).GetRandomValue(rng)...};
    }

    // template <>
    // inline ValueType get_tuple(const CorpusType& v, detail::seq<>) {}

    template <unsigned... I>
    inline ValueType get_tuple(const CorpusType& v, detail::seq<I...>) const {
        return ValueType{std::get<I>(inner_domains).GetValue(std::get<I>(v))...};
    }

    // template <>
    // inline CorpusType from_tuple(const ValueType& v, detail::seq<>) {}

    template <unsigned... I>
    inline CorpusType from_tuple(const ValueType& v, detail::seq<I...>) const {
        return CorpusType{std::get<I>(inner_domains).GetValue(std::get<I>(v))...};
    }

public:
    std::tuple<Inner...> inner_domains;

    AggregateOf(Inner&&... inner) : inner_domains(std::make_tuple(std::move(inner)...)) {}


    ValueType GetRandomValue(Rng& rng) override {
        return get_random(rng, detail::gen_seq<sizeof...(Inner)>{});
    }


    ValueType GetValue(const CorpusType& v) const override {
        return get_tuple(v, detail::gen_seq<sizeof...(Inner)>{});
    }


    CorpusType FromValue(const ValueType& v) const override {
        return from_tuple(v, detail::gen_seq<sizeof...(Inner)>{});
    }


    struct GetTupleDomain {
        Rng& rng;
        bool only_shrink;

        template <typename D, typename H>
        void operator()(const D& domain, H& value) {
            domain.Mutate(rng, value, only_shrink);
        }
    };

    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        unsigned index = rng.bounded(sizeof...(Inner));
        GetTupleDomain visitor{rng, only_shrink};
        detail::visit2_at(inner_domains, v, index, visitor);

        // detail::visit_at(inner_domains, index, [&](auto& domain) {
        //     detail::visit_at(v, index,
        //                      [&](auto& value) { domain.Mutate(rng, value, only_shrink); });
        // });
    }
};


#ifdef ZEROERR_ENABLE_PFR

template <typename T, typename... Inner>
auto StructOf(Inner&&... inner) {
    return AggregateOfImpl<T, Inner...>(std::move(inner)...);
}

#endif

template <typename... Inner>
auto TupleOf(Inner&&... inner) {
    return AggregateOf<std::tuple<typename Inner::ValueType...>, Inner...>(std::move(inner)...);
}


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP