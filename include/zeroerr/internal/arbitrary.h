#pragma once

#include "zeroerr/internal/config.h"


ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {

template <typename T>
class Arbitrary {

};

template <>
class Arbitrary<bool> {
};

template <>
class Arbitrary<int> {
};



} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP