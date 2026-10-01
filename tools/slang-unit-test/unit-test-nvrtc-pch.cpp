// unit-test-nvrtc-pch.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// Compile a compute entry point to PTX through NVRTC and return the emitted code. Each call uses a
// fresh session so the two compilations reach NVRTC as separate programs in one process — the shape
// in which NVRTC's process-global precompiled header applies.
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

// Guards that compiling the same entry point twice in one process produces byte-identical PTX. On
// NVRTC >= 12.8 the driver adds `-pch` for the second compile, so this specifically guards that
// enabling the precompiled header does not perturb the emitted code; on older NVRTC (no `-pch`) the
// same invariant still holds as a plain determinism check. It does not assert that a precompiled
// header was created or reused — NVRTC does not surface that through the emitted PTX (the
// invalidation test uses nvrtcGetPCHCreateStatus for that). NVRTC compiles to PTX without a GPU, so
// the test only needs a loadable NVRTC; otherwise it reports Ignored.
SLANG_UNIT_TEST(nvrtcPrecompiledHeader)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    const char* kernel = R"(
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = float(tid.x);
        }
        )";

    // Compile the same kernel twice in one process, then require byte-identical PTX: enabling
    // `-pch` for the second compilation must not change the result.
    ComPtr<slang::IBlob> code1;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileComputeToPTX(globalSession, kernel, code1)));
    SLANG_CHECK_ABORT(code1 != nullptr && code1->getBufferSize() != 0);

    ComPtr<slang::IBlob> code2;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileComputeToPTX(globalSession, kernel, code2)));
    SLANG_CHECK_ABORT(code2 != nullptr && code2->getBufferSize() != 0);

    SLANG_CHECK(code1->getBufferSize() == code2->getBufferSize());
    SLANG_CHECK(
        0 == memcmp(code1->getBufferPointer(), code2->getBufferPointer(), code1->getBufferSize()));
}
