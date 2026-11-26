// unit-test-stack-symbols.cpp

#include "core/slang-stack-symbols.h"
#include "core/slang-stack-trace.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// The result of the last capture. Storing to a volatile after the capture keeps the call from
// becoming a tail call, which would remove this function's frame.
static volatile int g_symbolTestFrameCount = 0;

// A function with external linkage and a name that no other code has, so that the test can
// look for it in a resolved name. It stays out of line so that it owns a stack frame.
SLANG_NO_INLINE void slangUnitTestStackSymbolsCaptureHere(StackTrace& outTrace)
{
    StackTraceUtil::capture(0, outTrace);
    g_symbolTestFrameCount = outTrace.frameCount;
}

SLANG_UNIT_TEST(stackSymbolsNameTheFunction, NoRenderApi)
{
    StackTrace trace;
    slangUnitTestStackSymbolsCaptureHere(trace);

    StackSymbolResolver resolver;
    String name;
    if (!StackSymbolResolver::isEnabled())
    {
        // A default build prints addresses only.
        SLANG_CHECK(
            trace.frameCount == 0 || !resolver.findFunctionName(trace.returnAddresses[0], name));
        return;
    }

    // With names enabled, the innermost frame, a return address inside the helper, resolves to
    // the helper's demangled name.
    SLANG_CHECK_ABORT(trace.frameCount >= 1);
    SLANG_CHECK_ABORT(resolver.findFunctionName(trace.returnAddresses[0], name));
    SLANG_CHECK(name.indexOf(toSlice("slangUnitTestStackSymbolsCaptureHere")) >= 0);

    // An address on the stack is in no function.
    int local = 0;
    String stackName;
    SLANG_CHECK(!resolver.findFunctionName(&local, stackName));
}
