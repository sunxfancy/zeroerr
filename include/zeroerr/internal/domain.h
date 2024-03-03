#include "zeroerr/internal/config.h"

#include <cstdint>

ZEROERR_SUPPRESS_COMMON_WARNINGS_PUSH

namespace zeroerr {


template<typename T>
class Domain {


private:
    std::unique_ptr<TypedDomain> domain;
};





} // namespace zeroerr

ZEROERR_SUPPRESS_COMMON_WARNINGS_POP