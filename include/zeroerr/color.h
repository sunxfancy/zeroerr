#pragma once

#include "zeroerr/config.h"

namespace zeroerr
{

#ifdef ZEROERR_ALWAYS_COLORFUL
constexpr const char* reset = "\x1B[0m";
constexpr const char* red = "\x1B[31m";
constexpr const char* green = "\x1B[32m";
constexpr const char* yellow = "\x1B[33m";
constexpr const char* blue = "\x1B[34m";
constexpr const char* magenta = "\x1B[35m";
constexpr const char* cyan = "\x1B[36m";
constexpr const char* white = "\x1B[37m";
#elif defined(ZEROERR_DISABLE_COLORFUL)
constexpr const char* reset = "";
constexpr const char* red = "";
constexpr const char* green = "";
constexpr const char* yellow = "";
constexpr const char* blue = "";
constexpr const char* magenta = "";
constexpr const char* cyan = "";
constexpr const char* white = "";
#else
extern const char* reset;
extern const char* red;
extern const char* green;
extern const char* yellow;
extern const char* blue;
extern const char* magenta;
extern const char* cyan;
extern const char* white;
    
extern void disableColorOutput();
extern void enableColorOutput();

enum OutputStream
{
    STDOUT,
    STDERR
};
extern bool isTerminalOutput(OutputStream stream);


#endif

} // namespace zeroerr
