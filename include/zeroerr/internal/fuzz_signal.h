#pragma once
#include <exception>
namespace zeroerr {
class FuzzFinishedException : public std::exception {
public:
    const char* what() const noexcept override { return "Fuzzing finished"; }
};
}  // namespace zeroerr
