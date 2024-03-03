#pragma once

#include "zeroerr/internal/config.h"
#include "zeroerr/domains/domain.h"

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T>
class Arbitrary {

};

template <>
class Arbitrary<bool> {
    using ValueType = bool;
    using CorpusType = uint64_t;

    
};

template <>
class Arbitrary<int> {
};



Arbitrary<int> InRange(int min, int max) {
    return Arbitrary<int>();
}


} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP