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
    using ValueType  = T;
    using CorpusType = std::tuple<typename Inner::CorpusType...>;

    std::tuple<Inner...> inner_domains;
public:
    AggregateOf(Inner&&... inner) : inner_domains(std::make_tuple(std::forward(inner)...)) {}

    ValueType GetRandomValue(Rng& rng) override {
        return elements;
    }

    template <unsigned... I>
    inline ValueType get_tuple(const CorpusType& v, detail::seq<I...>) {
        return ValueType{std::get<I>(inner_domains).GetValue(std::get<I>(v))...};
    }

    ValueType GetValue(const CorpusType& v) const override {
        return get_tuple(v, detail::gen_seq<sizeof...(Inner)>{});
    }

    template <unsigned... I>
    inline CorpusType from_tuple(const ValueType& v, detail::seq<I...>) {
        return CorpusType{std::get<I>(inner_domains).GetValue(std::get<I>(v))...};
    }

    CorpusType FromValue(const ValueType& v) const override {
        return from_tuple(v, detail::gen_seq<sizeof...(Inner)>{});
    }


    void Mutate(Rng& rng, CorpusType& v, bool only_shrink) const override {
        unsigned index = rng.bounded(sizeof...(Inner));
        visit_at(inner_domains, index, [&](const auto& domain) {
            visit_at(v, index, [&](auto& value) {
                domain.Mutate(rng, value, only_shrink);
            });
        });
    }
};


#ifdef ZEROERR_ENABLE_PFR

template<typename T, typename... Inner>
auto StructOf(Inner&&... inner) {
    return AggregateOfImpl<T, Inner...>(std::move(inner)...);
};

#endif


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP