// unit-test-stack-trace.cpp

#include "core/slang-stack-trace.h"
#include "unit-test/slang-unit-test.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

using namespace Slang;

// The result of the last `_captureHereAndInCaller` call. Storing to a volatile after the last
// call gives that function an observable action that follows the call.
static volatile int g_capturedFrameCount = 0;

/// Return the address that this call returns to, which is a point inside the caller.
static SLANG_NO_INLINE void* _getReturnAddressOfThisCall()
{
#if defined(_MSC_VER)
    return _ReturnAddress();
#else
    return __builtin_extract_return_addr(__builtin_return_address(0));
#endif
}

// Capture one trace that starts in this function and one that starts in its caller, and report
// an address inside this function.
//
// The function stays out of line so that it owns a stack frame of its own. The volatile store
// after the last call keeps that frame alive: without it, an optimizing compiler turns the last
// call into a tail call, and the function's frame is gone before the stack is walked.
static SLANG_NO_INLINE void _captureHereAndInCaller(
    StackTrace& outHere,
    StackTrace& outCaller,
    void*& outAddressInThisFunction)
{
    StackTraceUtil::capture(0, outHere);
    StackTraceUtil::capture(1, outCaller);
    outAddressInThisFunction = _getReturnAddressOfThisCall();
    g_capturedFrameCount = outHere.frameCount + outCaller.frameCount;
}

// Every package that we ship can capture a stack: the crash report depends on it. A platform
// that is added to this list without a working `capture` fails here instead of silently
// producing reports that have no stack.
SLANG_UNIT_TEST(stackTraceIsSupportedOnShippedPlatforms, NoRenderApi)
{
#if SLANG_WINDOWS_FAMILY || SLANG_OSX || defined(__GLIBC__)
    SLANG_CHECK(StackTraceUtil::isSupported());
#endif
}

SLANG_UNIT_TEST(stackTraceCapture, NoRenderApi)
{
    StackTrace here;
    StackTrace caller;
    void* addressInHelper = nullptr;
    _captureHereAndInCaller(here, caller, addressInHelper);

    if (!StackTraceUtil::isSupported())
    {
        SLANG_CHECK(here.frameCount == 0);
        SLANG_CHECK(caller.frameCount == 0);
        return;
    }

    SLANG_CHECK_ABORT(here.frameCount >= 2);
    SLANG_CHECK(here.frameCount <= StackTrace::kMaxFrameCount);
    SLANG_CHECK_ABORT(caller.frameCount >= 1);

    // The innermost frame is the return address of the first `capture` call in the helper. We
    // cannot name that address, but a later call in the helper returns to a point a few
    // instructions after it: 16 to 22 bytes on the platforms that we measured. A walk that
    // loses the innermost frames, or that keeps its own frame, lands in another function and
    // fails this check.
    static const ptrdiff_t kMaxDistance = 128;
    const ptrdiff_t distance = reinterpret_cast<const char*>(here.returnAddresses[0]) -
                               reinterpret_cast<const char*>(addressInHelper);
    const bool isInHelper = distance < 0 && distance > -kMaxDistance;
    SLANG_CHECK(isInHelper);
    if (!isInHelper)
    {
        // We can only see some platforms in CI, so the failure says what the walk returned.
        fprintf(
            stderr,
            "innermost frame is %lld bytes from the helper that called capture; "
            "%d frames captured\n",
            (long long)distance,
            here.frameCount);
        for (int i = 0; i < here.frameCount && i < 8; ++i)
        {
            StackFrameModule module;
            if (StackTraceUtil::findModule(here.returnAddresses[i], module))
                fprintf(
                    stderr,
                    "  #%d %s+0x%llx\n",
                    i,
                    module.moduleName,
                    (unsigned long long)module.offset);
            else
                fprintf(stderr, "  #%d ?\n", i);
        }
    }

    // Skipping one frame drops `_captureHereAndInCaller`, so the skipped trace starts at the
    // return address into this test, which is the second frame of the unskipped trace.
    SLANG_CHECK(caller.returnAddresses[0] == here.returnAddresses[1]);
}

SLANG_UNIT_TEST(stackTraceFindModule, NoRenderApi)
{
    // An address on the stack lies in no module on any platform.
    StackFrameModule module;
    int local = 0;
    SLANG_CHECK(!StackTraceUtil::findModule(&local, module));

    if (!StackTraceUtil::isSupported())
        return;

    StackTrace here;
    StackTrace caller;
    void* addressInHelper = nullptr;
    _captureHereAndInCaller(here, caller, addressInHelper);
    SLANG_CHECK_ABORT(here.frameCount >= 1);

    // The innermost frame is code of this test binary, so it resolves to a named module, and
    // the name carries no directory.
    SLANG_CHECK_ABORT(StackTraceUtil::findModule(here.returnAddresses[0], module));
    SLANG_CHECK(module.moduleName[0] != 0);
    SLANG_CHECK(strchr(module.moduleName, '/') == nullptr);
    SLANG_CHECK(strchr(module.moduleName, '\\') == nullptr);

    // A function of this file is in the same module as a return address into this file, and
    // because the module starts before both, both offsets are positive.
    StackFrameModule functionModule;
    const void* functionAddress = reinterpret_cast<const void*>(&_captureHereAndInCaller);
    SLANG_CHECK_ABORT(StackTraceUtil::findModule(functionAddress, functionModule));
    SLANG_CHECK(strcmp(functionModule.moduleName, module.moduleName) == 0);
    SLANG_CHECK(functionModule.offset > 0);
    SLANG_CHECK(module.offset > 0);
}
