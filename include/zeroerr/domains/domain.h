#include "zeroerr/internal/config.h"

#include "zeroerr/internal/serialization.h"
#include "zeroerr/internal/rng.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {


template<typename ValueType, typename CorpusType = ValueType>
class Domain {
public:
    virtual ~Domain() = default;

    virtual ValueType GetRandomValue(Rng& rng) = 0;
    ValueType GetValue(const CorpusType& v) const { return v; }
    CorpusType FromValue(const ValueType& v) const { return v; }

    virtual CorpusType ParseCorpus(IRObject v) const { return ToCorpus<CorpusType>(v); }
    virtual IRObject SerializeCorpus(const CorpusType& v) const { return FromCorpus(v); }

    virtual void Mutate(Rng& rng, CorpusType& v, bool only_shrink = false) const = 0;
    virtual unsigned CountNumberOfFields(CorpusType v) const { return 0; }
    virtual void MutateSelectedField(Rng& rng, CorpusType& v, unsigned field, bool only_shrink = false) const {}
};




} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP
