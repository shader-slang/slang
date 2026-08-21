// unit-test-nvrtc-pch.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string>

using namespace Slang;

// Compile a compute entry point to PTX through NVRTC and return the emitted code. `body` is a
// standalone Slang module; distinct bodies exercise the NVRTC precompiled-header (`-pch`)
// invalidation path (a changed translation unit must not reuse a stale PCH).
static SlangResult compileComputeToPTX(
    slang::IGlobalSession* globalSession,
    const char* body,
    ComPtr<slang::IBlob>& outCode)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", body, diagnosticBlob.writeRef());
    if (!module)
        return SLANG_FAIL;

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    if (!entryPoint)
        return SLANG_FAIL;

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> compositeProgram;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef()));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(
        compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef()));

    return linkedProgram->getEntryPointCode(0, 0, outCode.writeRef(), diagnosticBlob.writeRef());
}

// Regression test for issue #12622: the NVRTC driver enables automatic precompiled headers (`-pch`)
// on NVRTC 12.8+ from the second compile onward in a process. This guards against two ways that
// could go wrong — PCH reuse corrupting output when the same source is compiled twice, and a
// changed translation unit reusing a stale PCH. It asserts output correctness, not timing (timing
// is machine-dependent; the measured 6.5x speedup is documented in the PR). NVRTC compiles to PTX
// without a GPU, so the only requirement is a loadable NVRTC of version >= 12.8; otherwise the test
// reports Ignored, matching the availability gate in unit-test-downstream-compiler-version.cpp.
SLANG_UNIT_TEST(nvrtcPrecompiledHeader)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    int major = 0;
    int minor = 0;
    if (SLANG_FAILED(globalSession->getDownstreamCompilerVersion(
            SLANG_PASS_THROUGH_NVRTC,
            &major,
            &minor)) ||
        major < 12 || (major == 12 && minor < 8))
    {
        // `-pch` is only added on NVRTC 12.8+; on older toolkits there is nothing to exercise.
        SLANG_IGNORE_TEST;
    }

    const char* kernelA = R"(
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = float(tid.x);
        }
        )";

    // Same source as kernelA but a different computation, so its correct PTX differs. Compiling it
    // after kernelA exercises the invalidation path: the prior PCH must not be reused for it.
    const char* kernelB = R"(
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = float(tid.x) * 2.0f + 1.0f;
        }
        )";

    // First compile establishes a baseline PTX for kernelA. (The shared global session may have
    // compiled through NVRTC already, so `-pch` may or may not be active on this first call — the
    // test asserts correctness either way.)
    ComPtr<slang::IBlob> codeA1;
    SLANG_CHECK(SLANG_SUCCEEDED(compileComputeToPTX(globalSession, kernelA, codeA1)));
    SLANG_CHECK(codeA1 != nullptr && codeA1->getBufferSize() != 0);

    // Second compile of the same source: the amortization guard guarantees `-pch` is enabled by
    // now, so NVRTC reuses the prelude PCH. The result must still be valid and identical to the
    // first.
    ComPtr<slang::IBlob> codeA2;
    SLANG_CHECK(SLANG_SUCCEEDED(compileComputeToPTX(globalSession, kernelA, codeA2)));
    SLANG_CHECK(codeA2 != nullptr && codeA2->getBufferSize() != 0);
    SLANG_CHECK(codeA1->getBufferSize() == codeA2->getBufferSize());
    SLANG_CHECK(
        0 ==
        memcmp(codeA1->getBufferPointer(), codeA2->getBufferPointer(), codeA1->getBufferSize()));

    // Third compile of a different kernel with `-pch` still enabled: NVRTC must produce PTX for the
    // new source rather than reuse anything stale. It must succeed and differ from kernelA's PTX.
    ComPtr<slang::IBlob> codeB;
    SLANG_CHECK(SLANG_SUCCEEDED(compileComputeToPTX(globalSession, kernelB, codeB)));
    SLANG_CHECK(codeB != nullptr && codeB->getBufferSize() != 0);
    const bool differs =
        codeB->getBufferSize() != codeA1->getBufferSize() ||
        0 != memcmp(codeB->getBufferPointer(), codeA1->getBufferPointer(), codeA1->getBufferSize());
    SLANG_CHECK(differs);
}
