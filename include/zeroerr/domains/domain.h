#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/rng.h"
#include "zeroerr/internal/serialization.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {


template <typename ValueType, typename CorpusType = ValueType>
class Domain {
public:
    virtual ~Domain() = default;

    virtual CorpusType GetRandomCorpus(Rng& rng) const = 0;
    virtual ValueType  GetRandomValue(Rng& rng) const { return GetValue(GetRandomCorpus(rng)); };

    virtual CorpusType FromValue(const ValueType& v) const = 0;
    virtual ValueType  GetValue(const CorpusType& v) const = 0;

    virtual CorpusType ParseCorpus(IRObject v) const { return IRObject::ToCorpus<CorpusType>(v); }
    virtual IRObject SerializeCorpus(const CorpusType& v) const { return IRObject::FromCorpus(v); }

    virtual void     Mutate(Rng& rng, CorpusType& v, bool only_shrink = false) const = 0;
    virtual void     MutateSelectedField(Rng& rng, CorpusType& v, unsigned field,
                                         bool only_shrink = false) const {}
    virtual unsigned CountNumberOfFields(CorpusType v) const { return 0; }
};

template <typename ValueType, typename CorpusType = ValueType>
class DomainConvertable : public Domain<ValueType, CorpusType> {
public:
    virtual ValueType  GetValue(const CorpusType& v) const { return v; }
    virtual CorpusType FromValue(const ValueType& v) const { return v; }
};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP
