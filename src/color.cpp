#include "zeroerr/color.h"
#include "zeroerr/threadsafe.h"

#ifdef ZEROERR_OS_UNIX
#include <unistd.h>
#endif

#ifdef ZEROERR_OS_WINDOWS
#include <Windows.h>
#endif


#if !defined(ZEROERR_ALWAYS_COLORFUL) && !defined(ZEROERR_DISABLE_COLORFUL)
namespace zeroerr {

static const char* _Reset      = "\x1b[0m";
static const char* _Bright     = "\x1b[1m";
static const char* _Dim        = "\x1b[2m";
static const char* _Underscore = "\x1b[4m";
static const char* _Blink      = "\x1b[5m";
static const char* _Reverse    = "\x1b[7m";
static const char* _Hidden     = "\x1b[8m";

static const char* _FgBlack   = "\x1b[30m";
static const char* _FgRed     = "\x1b[31m";
static const char* _FgGreen   = "\x1b[32m";
static const char* _FgYellow  = "\x1b[33m";
static const char* _FgBlue    = "\x1b[34m";
static const char* _FgMagenta = "\x1b[35m";
static const char* _FgCyan    = "\x1b[36m";
static const char* _FgWhite   = "\x1b[37m";

static const char* _BgBlack   = "\x1b[40m";
static const char* _BgRed     = "\x1b[41m";
static const char* _BgGreen   = "\x1b[42m";
static const char* _BgYellow  = "\x1b[43m";
static const char* _BgBlue    = "\x1b[44m";
static const char* _BgMagenta = "\x1b[45m";
static const char* _BgCyan    = "\x1b[46m";
static const char* _BgWhite   = "\x1b[47m";
ZEROERR_MUTEX(m);

const char* Reset      = _Reset;
const char* Bright     = _Bright;
const char* Dim        = _Dim;
const char* Underscore = _Underscore;
const char* Blink      = _Blink;
const char* Reverse    = _Reverse;
const char* Hidden     = _Hidden;

const char* FgBlack   = _FgBlack;
const char* FgRed     = _FgRed;
const char* FgGreen   = _FgGreen;
const char* FgYellow  = _FgYellow;
const char* FgBlue    = _FgBlue;
const char* FgMagenta = _FgMagenta;
const char* FgCyan    = _FgCyan;
const char* FgWhite   = _FgWhite;

const char* BgBlack   = _BgBlack;
const char* BgRed     = _BgRed;
const char* BgGreen   = _BgGreen;
const char* BgYellow  = _BgYellow;
const char* BgBlue    = _BgBlue;
const char* BgMagenta = _BgMagenta;
const char* BgCyan    = _BgCyan;
const char* BgWhite   = _BgWhite;

void disableColorOutput() {
    ZEROERR_LOCK(m);
    Reset      = "";
    Bright     = "";
    Dim        = "";
    Underscore = "";
    Blink      = "";
    Reverse    = "";
    Hidden     = "";

    FgBlack   = "";
    FgRed     = "";
    FgGreen   = "";
    FgYellow  = "";
    FgBlue    = "";
    FgMagenta = "";
    FgCyan    = "";
    FgWhite   = "";

    BgBlack   = "";
    BgRed     = "";
    BgGreen   = "";
    BgYellow  = "";
    BgBlue    = "";
    BgMagenta = "";
    BgCyan    = "";
    BgWhite   = "";
}

void enableColorOutput() {
    ZEROERR_LOCK(m);
    Reset      = _Reset;
    Bright     = _Bright;
    Dim        = _Dim;
    Underscore = _Underscore;
    Blink      = _Blink;
    Reverse    = _Reverse;
    Hidden     = _Hidden;

    FgBlack   = _FgBlack;
    FgRed     = _FgRed;
    FgGreen   = _FgGreen;
    FgYellow  = _FgYellow;
    FgBlue    = _FgBlue;
    FgMagenta = _FgMagenta;
    FgCyan    = _FgCyan;
    FgWhite   = _FgWhite;

    BgBlack   = _BgBlack;
    BgRed     = _BgRed;
    BgGreen   = _BgGreen;
    BgYellow  = _BgYellow;
    BgBlue    = _BgBlue;
    BgMagenta = _BgMagenta;
    BgCyan    = _BgCyan;
    BgWhite   = _BgWhite;
}

#ifdef ZEROERR_OS_UNIX
bool isTerminalOutput(OutputStream stream) {
    switch (stream) {
        case STDOUT: return isatty(fileno(stdout)) != 0;
        case STDERR: return isatty(fileno(stderr)) != 0;
        default: return false;
    }
}
#endif

#ifdef ZEROERR_OS_WINDOWS
bool isTerminalOutput(OutputStream stream) {
    switch (stream) {
        case STDOUT: return GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR;
        case STDERR: return GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_CHAR;
        default: return false;
    }
}
#endif

#ifndef ZEROERR_DISABLE_AUTO_INIT
static struct ColorInit {
    ColorInit() {
        if (isTerminalOutput(STDERR)) {
            enableColorOutput();
        }
    }
} colorInit;
#endif


}  // namespace zeroerr

#endif
