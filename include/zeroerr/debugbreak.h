/* Copyright (c) 2011-2021, Scott Tsai
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef DEBUG_BREAK_H
#define DEBUG_BREAK_H

#ifdef _MSC_VER

#define debug_break __debugbreak

#else

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_BREAK_USE_TRAP_INSTRUCTION 1
#define DEBUG_BREAK_USE_BULTIN_TRAP      2
#define DEBUG_BREAK_USE_SIGTRAP          3

#if defined(__i386__) || defined(__x86_64__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__inline__ static void trap_instruction(void) { __asm__ volatile("int $0x03"); }
#elif defined(__thumb__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
/* FIXME: handle __THUMB_INTERWORK__ */
__attribute__((always_inline)) __inline__ static void trap_instruction(void) {
    /* See 'arm-linux-tdep.c' in GDB source.
     * Both instruction sequences below work. */
#if 1
    /* 'eabi_linux_thumb_le_breakpoint' */
    __asm__ volatile(".inst 0xde01");
#else
    /* 'eabi_linux_thumb2_le_breakpoint' */
    __asm__ volatile(".inst.w 0xf7f0a000");
#endif

    /* Known problem:
     * After a breakpoint hit, can't 'stepi', 'step', or 'continue' in GDB.
     * 'step' would keep getting stuck on the same instruction.
     *
     * Workaround: use the new GDB commands 'debugbreak-step' and
     * 'debugbreak-continue' that become available
     * after you source the script from GDB:
     *
     * $ gdb -x debugbreak-gdb.py <... USUAL ARGUMENTS ...>
     *
     * 'debugbreak-step' would jump over the breakpoint instruction with
     * roughly equivalent of:
     * (gdb) set $instruction_len = 2
     * (gdb) tbreak *($pc + $instruction_len)
     * (gdb) jump   *($pc + $instruction_len)
     */
}
#elif defined(__arm__) && !defined(__thumb__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__attribute__((always_inline)) __inline__ static void trap_instruction(void) {
    /* See 'arm-linux-tdep.c' in GDB source,
     * 'eabi_linux_arm_le_breakpoint' */
    __asm__ volatile(".inst 0xe7f001f0");
    /* Known problem:
     * Same problem and workaround as Thumb mode */
}
#elif defined(__aarch64__) && defined(__APPLE__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_BULTIN_DEBUGTRAP
#elif defined(__aarch64__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__attribute__((always_inline)) __inline__ static void trap_instruction(void) {
    /* See 'aarch64-tdep.c' in GDB source,
     * 'aarch64_default_breakpoint' */
    __asm__ volatile(".inst 0xd4200000");
}
#elif defined(__powerpc__)
/* PPC 32 or 64-bit, big or little endian */
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__attribute__((always_inline)) __inline__ static void trap_instruction(void) {
    /* See 'rs6000-tdep.c' in GDB source,
     * 'rs6000_breakpoint' */
    __asm__ volatile(".4byte 0x7d821008");

    /* Known problem:
     * After a breakpoint hit, can't 'stepi', 'step', or 'continue' in GDB.
     * 'step' stuck on the same instruction ("twge r2,r2").
     *
     * The workaround is the same as ARM Thumb mode: use debugbreak-gdb.py
     * or manually jump over the instruction. */
}
#elif defined(__riscv)
/* RISC-V 32 or 64-bit, whether the "C" extension
 * for compressed, 16-bit instructions are supported or not */
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__attribute__((always_inline)) __inline__ static void trap_instruction(void) {
    /* See 'riscv-tdep.c' in GDB source,
     * 'riscv_sw_breakpoint_from_kind' */
    __asm__ volatile(".4byte 0x00100073");
}
#else
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_SIGTRAP
#endif


#ifndef DEBUG_BREAK_IMPL
#error "debugbreak.h is not supported on this target"
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_TRAP_INSTRUCTION
__attribute__((always_inline)) __inline__ static void debug_break(void) { trap_instruction(); }
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_BULTIN_DEBUGTRAP
__attribute__((always_inline)) __inline__ static void debug_break(void) { __builtin_debugtrap(); }
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_BULTIN_TRAP
__attribute__((always_inline)) __inline__ static void debug_break(void) { __builtin_trap(); }
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_SIGTRAP
#include <signal.h>
__attribute__((always_inline)) __inline__ static void debug_break(void) { raise(SIGTRAP); }
#else
#error "invalid DEBUG_BREAK_IMPL value"
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifdef _MSC_VER */


// Here is for checking the debugger is running

#include <fstream>

#ifdef IS_DEBUGGER_ACTIVE
__attribute__((always_inline)) __inline__ static bool isDebuggerActive() {
    return IS_DEBUGGER_ACTIVE();
}
#else  // IS_DEBUGGER_ACTIVE
#ifdef __linux__
class ErrnoGuard {
public:
    ErrnoGuard() : m_oldErrno(errno) {}
    ~ErrnoGuard() { errno = m_oldErrno; }

private:
    int m_oldErrno;
};
// See the comments in Catch2 for the reasoning behind this implementation:
// https://github.com/catchorg/Catch2/blob/v2.13.1/include/internal/catch_debugger.cpp#L79-L102
__attribute__((always_inline)) __inline__ static bool isDebuggerActive() {
    ErrnoGuard    guard;
    std::ifstream in("/proc/self/status");
    for (std::string line; std::getline(in, line);) {
        static const int PREFIX_LEN = 11;
        if (line.compare(0, PREFIX_LEN, "TracerPid:\t") == 0) {
            return line.length() > PREFIX_LEN && line[PREFIX_LEN] != '0';
        }
    }
    return false;
}
#elif defined(__APPLE__)
// The following function is taken directly from the following technical note:
// https://developer.apple.com/library/archive/qa/qa1361/_index.html
// Returns true if the current process is being debugged (either
// running under the debugger or has a debugger attached post facto).
__attribute__((always_inline)) __inline__ static bool isDebuggerActive() {
    int        mib[4];
    kinfo_proc info;
    size_t     size;
    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    info.kp_proc.p_flag = 0;
    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    // Call sysctl.
    size = sizeof(info);
    if (sysctl(mib, (sizeof(mib) / sizeof(*mib)), &info, &size, 0, 0) != 0) {
        std::cerr << "\nCall to sysctl failed - unable to determine if debugger is active **\n";
        return false;
    }
    // We're being debugged if the P_TRACED flag is set.
    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}
#elif defined(__MINGW32__) || defined(__MINGW64__)
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
__attribute__((always_inline)) __inline__  static bool isDebuggerActive() {
    return ::IsDebuggerPresent() != 0;
}
#elif defined(_MSC_VER)
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
inline __forceinline static bool isDebuggerActive() {
    return ::IsDebuggerPresent() != 0;
}
#else

__attribute__((always_inline)) __inline__ static bool isDebuggerActive() { return false; }
#endif
#endif  // IS_DEBUGGER_ACTIVE


#endif /* ifndef DEBUG_BREAK_H */