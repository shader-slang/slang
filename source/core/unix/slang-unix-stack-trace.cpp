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

// We keep `capture` out of line so that it always owns exactly one stack frame. The frame count
// that it removes from the front of the trace would otherwise depend on the optimization level.
/* static */ SLANG_NO_INLINE void StackTraceUtil::capture(int skipFrameCount, StackTrace& outTrace)
{
    if (skipFrameCount < 0)
        skipFrameCount = 0;
    if (skipFrameCount > kMaxSkipFrameCount)
        skipFrameCount = kMaxSkipFrameCount;

    // `backtrace` reports the frame of its own caller first, which is this function, so we
    // always remove one more frame than the caller asked for.
    const int hiddenFrameCount = skipFrameCount + 1;

    void* addresses[StackTrace::kMaxFrameCount + kMaxSkipFrameCount + 1];
    const int addressCount = ::backtrace(addresses, StackTrace::kMaxFrameCount + hiddenFrameCount);

    int frameCount = addressCount - hiddenFrameCount;
    if (frameCount < 0)
        frameCount = 0;

    memcpy(outTrace.returnAddresses, addresses + hiddenFrameCount, sizeof(void*) * frameCount);
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
