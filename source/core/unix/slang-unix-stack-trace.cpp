// slang-unix-stack-trace.cpp
#include "core/slang-stack-trace.h"

#include <string.h>

#if SLANG_HAS_BACKTRACE
#include <dlfcn.h>
#include <execinfo.h>
#endif

namespace Slang
{

/* static */ bool StackTraceUtil::isSupported()
{
    return SLANG_HAS_BACKTRACE != 0;
}

#if SLANG_HAS_BACKTRACE

// The number of frames that a `backtrace` implementation may report above our caller: this
// function, and frames of an interposed `backtrace`, such as the one that AddressSanitizer
// installs, which reports frames of its own interceptor.
static const int kMaxLeadingFrameCount = 8;

// We keep `capture` out of line so that it always owns exactly one stack frame, whose return
// address `__builtin_return_address(0)` gives us.
/* static */ SLANG_NO_INLINE void StackTraceUtil::capture(int skipFrameCount, StackTrace& outTrace)
{
    if (skipFrameCount < 0)
        skipFrameCount = 0;
    if (skipFrameCount > kMaxSkipFrameCount)
        skipFrameCount = kMaxSkipFrameCount;

    void* const returnAddress = __builtin_extract_return_addr(__builtin_return_address(0));

    void* addresses[StackTrace::kMaxFrameCount + kMaxSkipFrameCount + kMaxLeadingFrameCount];
    const int addressCount = ::backtrace(addresses, int(SLANG_COUNT_OF(addresses)));

    // The trace starts at the return address into our caller. We find that frame by its
    // address instead of assuming how many frames come before it, because that number depends
    // on which `backtrace` the process uses. If the address is not in the trace, we assume
    // the one frame of this function.
    int callerFrame = 1;
    for (int i = 0; i < addressCount && i < kMaxLeadingFrameCount; ++i)
    {
        if (addresses[i] == returnAddress)
        {
            callerFrame = i;
            break;
        }
    }

    const int firstFrame = callerFrame + skipFrameCount;
    int frameCount = addressCount - firstFrame;
    if (frameCount < 0)
        frameCount = 0;
    if (frameCount > StackTrace::kMaxFrameCount)
        frameCount = StackTrace::kMaxFrameCount;

    memcpy(outTrace.returnAddresses, addresses + firstFrame, sizeof(void*) * frameCount);
    outTrace.frameCount = frameCount;
}

/* static */ bool StackTraceUtil::findModule(const void* address, StackFrameModule& outModule)
{
    Dl_info info;
    if (!::dladdr(address, &info) || !info.dli_fbase)
        return false;

    outModule.offset = uintptr_t(address) - uintptr_t(info.dli_fbase);

    // The loader reports the path that the module was loaded from. We keep only the file name.
    const char* name = info.dli_fname ? info.dli_fname : "";
    for (const char* cursor = name; *cursor; ++cursor)
    {
        if (*cursor == '/')
            name = cursor + 1;
    }

    size_t nameLength = strlen(name);
    if (nameLength > size_t(StackFrameModule::kMaxModuleNameLength))
        nameLength = size_t(StackFrameModule::kMaxModuleNameLength);
    memcpy(outModule.moduleName, name, nameLength);
    outModule.moduleName[nameLength] = 0;
    return true;
}

#else

// A C library without `backtrace`, such as musl or Emscripten's, gives us no portable way to
// walk the stack, so a trace on these platforms is empty and no address resolves.

/* static */ void StackTraceUtil::capture(int skipFrameCount, StackTrace& outTrace)
{
    SLANG_UNUSED(skipFrameCount);
    outTrace.frameCount = 0;
}

/* static */ bool StackTraceUtil::findModule(const void* address, StackFrameModule& outModule)
{
    SLANG_UNUSED(address);
    SLANG_UNUSED(outModule);
    return false;
}

#endif

} // namespace Slang
