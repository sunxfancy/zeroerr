#pragma once

#include "zeroerr/internal/config.h"

namespace zeroerr {

#ifdef ZEROERR_ALWAYS_COLORFUL
constexpr const char* Reset      = "\x1b[0m";
constexpr const char* Bright     = "\x1b[1m";
constexpr const char* Dim        = "\x1b[2m";
constexpr const char* Underscore = "\x1b[4m";
constexpr const char* Blink      = "\x1b[5m";
constexpr const char* Reverse    = "\x1b[7m";
constexpr const char* Hidden     = "\x1b[8m";

constexpr const char* FgBlack   = "\x1b[30m";
constexpr const char* FgRed     = "\x1b[31m";
constexpr const char* FgGreen   = "\x1b[32m";
constexpr const char* FgYellow  = "\x1b[33m";
constexpr const char* FgBlue    = "\x1b[34m";
constexpr const char* FgMagenta = "\x1b[35m";
constexpr const char* FgCyan    = "\x1b[36m";
constexpr const char* FgWhite   = "\x1b[37m";

constexpr const char* BgBlack   = "\x1b[40m";
constexpr const char* BgRed     = "\x1b[41m";
constexpr const char* BgGreen   = "\x1b[42m";
constexpr const char* BgYellow  = "\x1b[43m";
constexpr const char* BgBlue    = "\x1b[44m";
constexpr const char* BgMagenta = "\x1b[45m";
constexpr const char* BgCyan    = "\x1b[46m";
constexpr const char* BgWhite   = "\x1b[47m";

#elif defined(ZEROERR_DISABLE_COLORFUL)
constexpr const char* Reset      = "";
constexpr const char* Bright     = "";
constexpr const char* Dim        = "";
constexpr const char* Underscore = "";
constexpr const char* Blink      = "";
constexpr const char* Reverse    = "";
constexpr const char* Hidden     = "";

constexpr const char* FgBlack   = "";
constexpr const char* FgRed     = "";
constexpr const char* FgGreen   = "";
constexpr const char* FgYellow  = "";
constexpr const char* FgBlue    = "";
constexpr const char* FgMagenta = "";
constexpr const char* FgCyan    = "";
constexpr const char* FgWhite   = "";

constexpr const char* BgBlack   = "";
constexpr const char* BgRed     = "";
constexpr const char* BgGreen   = "";
constexpr const char* BgYellow  = "";
constexpr const char* BgBlue    = "";
constexpr const char* BgMagenta = "";
constexpr const char* BgCyan    = "";
constexpr const char* BgWhite   = "";
#else
extern const char* Reset;
extern const char* Bright;
extern const char* Dim;
extern const char* Underscore;
extern const char* Blink;
extern const char* Reverse;
extern const char* Hidden;

extern const char* FgBlack;
extern const char* FgRed;
extern const char* FgGreen;
extern const char* FgYellow;
extern const char* FgBlue;
extern const char* FgMagenta;
extern const char* FgCyan;
extern const char* FgWhite;

extern const char* BgBlack;
extern const char* BgRed;
extern const char* BgGreen;
extern const char* BgYellow;
extern const char* BgBlue;
extern const char* BgMagenta;
extern const char* BgCyan;
extern const char* BgWhite;

/**
 * @brief Global function to disable colorful output.
 */
extern void disableColorOutput();

/**
 * @brief Global function to enable colorful output.
 */
extern void enableColorOutput();

#endif

}  // namespace zeroerr
