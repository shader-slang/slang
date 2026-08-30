#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Recovering the loaded compiler's path is a pure metadata query, so this runs without a GPU: it
// only needs the downstream shared library to load, not a device. Availability of any given
// pass-through varies by machine, so each case below is gated the same way the codegen tests gate
// their pass-throughs.
SLANG_UNIT_TEST(getDownstreamCompilerPath)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    // NONE is not a real compiler, so it is rejected before any discovery.
    {
        ComPtr<ISlangBlob> path;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerPath(SLANG_PASS_THROUGH_NONE, path.writeRef()) ==
            SLANG_E_NOT_FOUND);
    }

    // An out-of-range pass-through value must be rejected at the boundary, not used to index the
    // per-type compiler arrays.
    {
        ComPtr<ISlangBlob> path;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerPath(
                SlangPassThrough(SLANG_PASS_THROUGH_COUNT_OF),
                path.writeRef()) == SLANG_E_NOT_FOUND);
    }

    // Gate the NVRTC path on availability the same way the version test does, so this test passes
    // both on machines that have NVRTC and those that do not (no GPU is required merely to load the
    // NVRTC library and recover its path).
    const bool nvrtcAvailable =
        SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC));

    ComPtr<ISlangBlob> nvrtcPath;
    const SlangResult result =
        globalSession->getDownstreamCompilerPath(SLANG_PASS_THROUGH_NVRTC, nvrtcPath.writeRef());

    if (nvrtcAvailable)
    {
        // NVRTC is a shared library, so its path is recoverable from a held symbol.
        SLANG_CHECK(result == SLANG_OK);
        SLANG_CHECK(nvrtcPath != nullptr);
        SLANG_CHECK(nvrtcPath->getBufferSize() > 0);
    }
    else
    {
        SLANG_CHECK(result == SLANG_E_NOT_FOUND);
    }

    // glslang is a bundled, GPU-independent shared library that loads on CI runners without a
    // device. When loadable, its path must be recoverable (SLANG_OK + non-empty), exercising the
    // shared-library recovery path for a compiler that reports no numeric version.
    if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_GLSLANG)))
    {
        ComPtr<ISlangBlob> glslangPath;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerPath(
                SLANG_PASS_THROUGH_GLSLANG,
                glslangPath.writeRef()) == SLANG_OK);
        SLANG_CHECK(glslangPath != nullptr);
        SLANG_CHECK(glslangPath->getBufferSize() > 0);
    }
}
