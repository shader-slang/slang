// slang-win-stack-trace.cpp
#include "core/slang-stack-trace.h"

#include <windows.h>

namespace Slang
{

/* static */ bool StackTraceUtil::isSupported()
{
    return true;
}

#if SLANG_PROCESSOR_X86_64 || SLANG_PROCESSOR_ARM_64

// On 64-bit Windows every function that changes the stack has unwind data, and the system can
// step a register context from a function to its caller with it. We walk the stack that way and
// not with `RtlCaptureStackBackTrace`, for two reasons. In our optimized ARM64 build, the first
// frame that `RtlCaptureStackBackTrace` returned was not in the module of our caller. And a
// crash handler has to start the walk from the context of the fault, which only a walk over a
// context can do.

static DWORD64 _getProgramCounter(const CONTEXT& context)
{
#if SLANG_PROCESSOR_X86_64
    return context.Rip;
#else
    return context.Pc;
#endif
}

static DWORD64 _getStackPointer(const CONTEXT& context)
{
#if SLANG_PROCESSOR_X86_64
    return context.Rsp;
#else
    return context.Sp;
#endif
}

/// Step `context` from the function that it is in to the caller of that function.
///
/// Return false at the outermost frame, where there is no caller: on x64 the unwinder leaves a
/// null program counter there, and on ARM64 it leaves the context unchanged.
static bool _unwindToCaller(CONTEXT& context)
{
    const DWORD64 programCounter = _getProgramCounter(context);
    const DWORD64 stackPointer = _getStackPointer(context);

    DWORD64 imageBase = 0;
    PRUNTIME_FUNCTION function = ::RtlLookupFunctionEntry(programCounter, &imageBase, nullptr);
    if (function)
    {
        void* handlerData = nullptr;
        DWORD64 establisherFrame = 0;
        ::RtlVirtualUnwind(
            UNW_FLAG_NHANDLER,
            imageBase,
            programCounter,
            function,
            &context,
            &handlerData,
            &establisherFrame,
            nullptr);
    }
    else
    {
        // A function without unwind data is a leaf that left the stack as it found it, so its
        // return address is where the call put it: on top of the stack on x64, and in the
        // link register on ARM64.
#if SLANG_PROCESSOR_X86_64
        context.Rip = *reinterpret_cast<const DWORD64*>(context.Rsp);
        context.Rsp += sizeof(DWORD64);
#else
        context.Pc = context.Lr;
#endif
    }

    const DWORD64 callerProgramCounter = _getProgramCounter(context);
    if (callerProgramCounter == 0)
        return false;
    return callerProgramCounter != programCounter || _getStackPointer(context) != stackPointer;
}

// We keep `capture` out of line so that it always owns exactly one stack frame: the first step
// of the walk leaves this function and arrives at the return address in its caller.
/* static */ SLANG_NO_INLINE void StackTraceUtil::capture(int skipFrameCount, StackTrace& outTrace)
{
    if (skipFrameCount < 0)
        skipFrameCount = 0;
    if (skipFrameCount > kMaxSkipFrameCount)
        skipFrameCount = kMaxSkipFrameCount;

    CONTEXT context;
    ::RtlCaptureContext(&context);

    // The bound on the number of steps ends the walk even if a damaged stack sends the
    // unwinder around a cycle.
    int frameCount = 0;
    const int maxStepCount = StackTrace::kMaxFrameCount + skipFrameCount;
    for (int step = 0; step < maxStepCount; ++step)
    {
        if (!_unwindToCaller(context))
            break;
        if (step < skipFrameCount)
            continue;
        outTrace.returnAddresses[frameCount++] =
            reinterpret_cast<void*>(_getProgramCounter(context));
    }
    outTrace.frameCount = frameCount;
}

#else

// 32-bit Windows has no table-based unwinder, so there we take the trace that the system walks.
//
// We keep `capture` out of line so that it always owns exactly one stack frame. The frame count
// that it removes from the front of the trace would otherwise depend on the optimization level.
/* static */ SLANG_NO_INLINE void StackTraceUtil::capture(int skipFrameCount, StackTrace& outTrace)
{
    if (skipFrameCount < 0)
        skipFrameCount = 0;
    if (skipFrameCount > kMaxSkipFrameCount)
        skipFrameCount = kMaxSkipFrameCount;

    // The first frame that the system reports is this function, so we always remove one more
    // frame than the caller asked for.
    const ULONG hiddenFrameCount = ULONG(skipFrameCount) + 1;

    outTrace.frameCount = int(::RtlCaptureStackBackTrace(
        hiddenFrameCount,
        ULONG(StackTrace::kMaxFrameCount),
        outTrace.returnAddresses,
        nullptr));
}

#endif

/* static */ bool StackTraceUtil::findModule(const void* address, StackFrameModule& outModule)
{
    // A module handle on Windows is the address at which the module is loaded.
    HMODULE module = nullptr;
    const DWORD flags =
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    if (!::GetModuleHandleExW(flags, reinterpret_cast<LPCWSTR>(address), &module) || !module)
        return false;

    outModule.offset = uintptr_t(address) - uintptr_t(module);
    outModule.moduleName[0] = 0;

    // We read the path into a fixed buffer because this code also runs after a crash, where it
    // must not allocate. A path that does not fit is cut at its end, which is where the file
    // name is, so we leave the name empty in that case instead of reporting a wrong one.
    static const DWORD kMaxPathLength = 1024;
    wchar_t path[kMaxPathLength];
    const DWORD pathLength = ::GetModuleFileNameW(module, path, kMaxPathLength);
    if (pathLength == 0 || pathLength >= kMaxPathLength)
        return true;

    DWORD nameStart = 0;
    for (DWORD i = 0; i < pathLength; ++i)
    {
        if (path[i] == L'\\' || path[i] == L'/')
            nameStart = i + 1;
    }

    // The module names that a report needs are ASCII. A character outside ASCII becomes `?`,
    // which keeps the conversion free of failure cases and of locale state.
    int nameLength = 0;
    for (DWORD i = nameStart; i < pathLength && nameLength < StackFrameModule::kMaxModuleNameLength;
         ++i)
    {
        const wchar_t c = path[i];
        outModule.moduleName[nameLength++] = (c < 0x80) ? char(c) : '?';
    }
    outModule.moduleName[nameLength] = 0;
    return true;
}

} // namespace Slang
