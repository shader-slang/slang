// unit-test-specialized-component-hash.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that calling getHash on components specialized with Expr works fine.

SLANG_UNIT_TEST(specializedComponentHash)
{
    // Shader with generic entrypoint that can be specialized with an expr.
    const char* userSourceBody = R"(
        [shader("compute")]
        [numthreads(4, 1, 1)]
        void computeMain<int k, T>(
            uint tid: SV_DispatchThreadID,
            uniform RWStructuredBuffer<float> output)
        {
            output[tid] = k + sizeof(T);
        }
        )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CUDA_SOURCE;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    auto floatType = module->getLayout()->findTypeByName("float");
    SLANG_CHECK_ABORT(floatType != nullptr);

    slang::SpecializationArg specArgs[] = {
        slang::SpecializationArg::fromExpr("3"),
        slang::SpecializationArg::fromType(floatType)};
    ComPtr<slang::IComponentType> specializedEntrypoint;
    entryPoint->specialize(specArgs, 2, specializedEntrypoint.writeRef());

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, specializedEntrypoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> hashBlob;
    linkedProgram->getEntryPointHash(0, 0, hashBlob.writeRef());
    SLANG_CHECK_ABORT(hashBlob.get() != nullptr);
    SLANG_CHECK_ABORT(hashBlob->getBufferSize() != 0);
}
