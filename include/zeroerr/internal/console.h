#pragma once

#include "zeroerr/internal/config.h"

namespace zeroerr {

enum OutputStream { STDOUT, STDERR };
extern bool isTerminalOutput(OutputStream stream);


struct TerminalSize {
    int width;
    int height;
};
TerminalSize getTerminalSize();

}  // namespace zeroerr
