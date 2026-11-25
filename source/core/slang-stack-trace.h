// slang-stack-trace.h
#ifndef SLANG_CORE_STACK_TRACE_H
#define SLANG_CORE_STACK_TRACE_H

#include "slang-common.h"

#include <stdint.h>

namespace Slang
{

/// The return addresses of one thread's call stack, innermost call first.
///
/// We store the addresses in a fixed array because a stack trace is captured on failure paths:
/// after an internal compiler error, and from a fatal-signal handler. Code on those paths
/// cannot rely on the heap, which may be the thing that is corrupted.
struct StackTrace
{
    static const int kMaxFrameCount = 64;

    void* returnAddresses[kMaxFrameCount];
    int frameCount = 0;
};

/// The position of one return address inside the binary that contains it.
///
/// A crash report prints a frame as `moduleName+0xoffset`. That pair is the same on every run of
/// the same build, because it does not depend on where the loader placed the module, so a
/// maintainer can turn it into a function name with the debug files of that build. For example,
/// `libslang-compiler.so+0x3a1786` resolves with `addr2line -f -e libslang-compiler.so 0x3a1786`.
struct StackFrameModule
{
    static const int kMaxModuleNameLength = 63;

    /// The file name of the module without its directory, so that a report never carries a path
    /// from the user's machine. The name is empty if the loader cannot name the module.
    char moduleName[kMaxModuleNameLength + 1];

    /// The distance of the address from the start of the loaded module.
    uintptr_t offset;
};

struct StackTraceUtil
{
    /// The largest `skipFrameCount` that `capture` honors.
    static const int kMaxSkipFrameCount = 16;

    /// Return true if `capture` can produce frames on this platform.
    static bool isSupported();

    /// Capture the call stack of the calling thread into `outTrace`.
    ///
    /// `skipFrameCount` is the number of innermost callers to leave out. With 0, the first frame
    /// is the return address in the function that called `capture`; with 1, it is the return
    /// address in that function's caller. On a platform without support, `outTrace` has no
    /// frames.
    ///
    /// A `skipFrameCount` outside `[0, kMaxSkipFrameCount]` is clamped instead of asserted,
    /// because an assert on this path would capture a stack trace again.
    static void capture(int skipFrameCount, StackTrace& outTrace);

    /// Find the module that contains `address` and the offset of `address` inside that module.
    ///
    /// Return false, and leave `outModule` unspecified, if no loaded module contains `address`.
    static bool findModule(const void* address, StackFrameModule& outModule);
};

} // namespace Slang

#endif
