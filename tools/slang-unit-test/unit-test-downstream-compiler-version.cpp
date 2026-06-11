// unit-test-downstream-compiler-version.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Test the IGlobalSession::getDownstreamCompilerVersion API (issue #11552). The method reports the
// version of the downstream compiler Slang will actually load for a given pass-through, using the
// same lazy discovery that compilation uses. This is purely a metadata query, so it runs without a
// GPU: when the downstream compiler is loadable we expect a real version; otherwise the method
// returns SLANG_E_NOT_FOUND, exactly like checkPassThroughSupport.
SLANG_UNIT_TEST(getDownstreamCompilerVersion)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    // SLANG_PASS_THROUGH_NONE is never a loadable compiler. Also verify null out-params are
    // tolerated on this always-reachable path (the NVRTC null-out check below only runs where
    // NVRTC is installed).
    {
        int major = -1;
        int minor = -1;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NONE, &major, &minor) ==
            SLANG_E_NOT_FOUND);
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(
                SLANG_PASS_THROUGH_NONE,
                nullptr,
                nullptr) == SLANG_E_NOT_FOUND);
    }

    // An out-of-range pass-through value must be rejected at the boundary, not used to index the
    // per-type compiler arrays.
    {
        int major = -1;
        int minor = -1;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(
                SlangPassThrough(SLANG_PASS_THROUGH_COUNT_OF),
                &major,
                &minor) == SLANG_E_NOT_FOUND);
    }

    // Gate the NVRTC path on availability the same way the CUDA codegen tests do, so this test
    // passes both on machines that have NVRTC and those that do not (no GPU is required merely to
    // load the NVRTC library and read its version).
    const bool nvrtcAvailable =
        SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC));

    int major = -1;
    int minor = -1;
    const SlangResult result =
        globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NVRTC, &major, &minor);

    if (nvrtcAvailable)
    {
        // The loaded NVRTC always reports a real version (set from nvrtcVersion at load time),
        // so the major component is non-zero (e.g. 12 for CUDA 12.x).
        SLANG_CHECK(result == SLANG_OK);
        SLANG_CHECK(major > 0);

        // Null out-params must be tolerated.
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(
                SLANG_PASS_THROUGH_NVRTC,
                nullptr,
                nullptr) == SLANG_OK);
    }
    else
    {
        SLANG_CHECK(result == SLANG_E_NOT_FOUND);
    }

    // Exercise the documented "loaded but versionless" clause: a bundled, GPU-independent compiler
    // that loads but exposes no numeric version must still succeed with (0,0) — it must NOT be
    // conflated with SLANG_E_NOT_FOUND. glslang leaves getDesc().version at (0,0) and is loadable
    // on CI runners without a device.
    if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_GLSLANG)))
    {
        int gMajor = -1;
        int gMinor = -1;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(
                SLANG_PASS_THROUGH_GLSLANG,
                &gMajor,
                &gMinor) == SLANG_OK);
        SLANG_CHECK(gMajor == 0 && gMinor == 0);
    }
}
