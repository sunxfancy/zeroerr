#pragma once
#include "zeroerr/internal/config.h"

#include "zeroerr/internal/rng.h"
#include "zeroerr/internal/serialization.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {


/**
 * @brief Domain class for generating random values of a specific type.
 * @tparam ValueType The type of the value to generate.
 * @tparam CorpusType The type of the corpus stored in the domain.
 * Here is an example. If you want to generate an list of intergers, 
 * but will store the list in a vector, then ValueType will be 
 * std::list<int> and CorpusType will be std::vector<int>.
 */
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
    // virtual void     MutateSelectedField(Rng& rng, CorpusType& v, unsigned field,
    //                                      bool only_shrink = false) const {}
    // virtual unsigned CountNumberOfFields(CorpusType v) const { return 0; }
};


/**
 * @brief DomainConvertable is a base class for domains that can be converted to and from a ValueType
 * 
 * This class provides default implementations for the GetValue and FromValue methods.
 * It is used to convert between the corpus types and the value types.
 */
template <typename ValueType, typename CorpusType = ValueType>
class DomainConvertable : public Domain<ValueType, CorpusType> {
public:
    virtual ValueType  GetValue(const CorpusType& v) const { return v; }
    virtual CorpusType FromValue(const ValueType& v) const { return v; }
};


}  // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP
