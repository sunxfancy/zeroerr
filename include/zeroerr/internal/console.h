#pragma once

#include "zeroerr/internal/config.h"

namespace zeroerr {

enum OutputStream { STDOUT, STDERR };
extern bool isTerminalOutput(OutputStream stream);


struct TerminalSize {
    unsigned width;
    unsigned height;
};
TerminalSize getTerminalSize();

}  // namespace zeroerr
