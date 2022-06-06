#include "zeroerr/color.h"

#ifndef ZEROERR_NO_THREAD_SAFE
#include <mutex>
#endif

#ifdef ZEROERR_OS_UNIX
#include <unistd.h>
#endif

#ifdef ZEROERR_OS_WINDOWS
#include <Windows.h>
#endif


#if !defined(ZEROERR_ALWAYS_COLORFUL) && !defined(ZEROERR_DISABLE_COLORFUL)
namespace zeroerr
{
    
static const char* _reset = "\x1B[0m";
static const char* _red = "\x1B[31m";
static const char* _green = "\x1B[32m";
static const char* _yellow = "\x1B[33m";
static const char* _blue = "\x1B[34m";
static const char* _magenta = "\x1B[35m";
static const char* _cyan = "\x1B[36m";
static const char* _white = "\x1B[37m";
ZEROERR_MUTEX(m);

const char* reset = _reset;
const char* red = _red; 
const char* green = _green; 
const char* yellow = _yellow; 
const char* blue = _blue; 
const char* magenta = _magenta; 
const char* cyan = _cyan; 
const char* white = _white;

void disableColorOutput() {
    ZEROERR_LOCK(m);
    reset = "";
    red = "";
    green = "";
    yellow = "";
    blue = "";
    magenta = "";
    cyan = "";
    white = "";
}

void enableColorOutput() {
    ZEROERR_LOCK(m);
    reset = _reset;
    red = _red;
    green = _green;
    yellow = _yellow;
    blue = _blue;
    magenta = _magenta;
    cyan = _cyan;
    white = _white;
}

#ifdef ZEROERR_OS_UNIX
bool isTerminalOutput(OutputStream stream) {
    switch (stream) {
        case STDOUT:
            return isatty(fileno(stdout)) != 0;
        case STDERR:
            return isatty(fileno(stderr)) != 0;
        default:
            return false;
    }
}
#endif 

#ifdef ZEROERR_OS_WINDOWS
bool isTerminalOutput(OutputStream stream) {
    switch (stream) {
        case STDOUT:
            return GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR;
        case STDERR:
            return GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_CHAR;
        default:
            return false;
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



} // namespace zeroerr

#endif
