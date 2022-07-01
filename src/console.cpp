#include "zeroerr/internal/console.h"
#include <cstdio>

#ifdef ZEROERR_OS_UNIX
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef ZEROERR_OS_WINDOWS
#include <Windows.h>
#endif

namespace zeroerr {

#ifdef ZEROERR_OS_UNIX
bool isTerminalOutput(OutputStream stream) {
    switch (stream) {
        case STDOUT: return isatty(fileno(stdout)) != 0;
        case STDERR: return isatty(fileno(stderr)) != 0;
        default: return false;
    }
}

TerminalSize getTerminalWidth() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return {80, 24};
    }
    return {ws.ws_col, ws.ws_row};
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

TerminalSize getTerminalWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return {csbi.dwSize.X, csbi.dwSize.Y};
}

#endif


}  // namespace zeroerr