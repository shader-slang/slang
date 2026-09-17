// unit-test-translation-unit-import.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the IModule::findAndCheckEntryPoint API supports discovering
// entrypoints without a [shader] attribute.

SLANG_UNIT_TEST(findAndCheckEntryPoint)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        float4 fragMain(float4 pos:SV_Position) : SV_Target
        {
            return pos;
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "fragMain",
        SLANG_STAGE_FRAGMENT,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(code != nullptr);
    SLANG_CHECK(code->getBufferSize() != 0);
}

// Loading the structural ray-tracing module in one compilation must not reinterpret declarations
// from an independent module in the same session. The trusted-declaration registry is
// linkage-wide, but structural entry-point lookup is valid only in a module whose dependency graph
// contains `slang.raytracing`.
SLANG_UNIT_TEST(structuralRayTracingEntryPointLookupIsModuleLocal)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::CompilerOptionEntry experimentalFeature = {};
    experimentalFeature.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeature.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeature.value.intValue0 = 1;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &experimentalFeature;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto importingModule = session->loadModuleFromSourceString(
        "StructuralImporter",
        "StructuralImporter.slang",
        "import slang.raytracing;",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(importingModule != nullptr);

    diagnostics.setNull();
    auto unrelatedModule = session->loadModuleFromSourceString(
        "UnrelatedModule",
        "UnrelatedModule.slang",
        "struct Plain {}",
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(unrelatedModule != nullptr);

    diagnostics.setNull();
    ComPtr<slang::IEntryPoint> entryPoint;
    auto result = unrelatedModule->findAndCheckEntryPoint(
        "Plain",
        SLANG_STAGE_MISS,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(entryPoint == nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);

    auto diagnosticText = UnownedStringSlice(
        (const char*)diagnostics->getBufferPointer(),
        diagnostics->getBufferSize());
    SLANG_CHECK(
        diagnosticText.indexOf(toSlice("no function found matching entry point name 'Plain'")) !=
        -1);
    SLANG_CHECK(diagnosticText.indexOf(toSlice("structural ray-tracing")) == -1);
}

// This test reproduces issue #6507, where it was noticed that compilation of
// tests/compute/simple.slang for PTX target generates invalid code.
// TODO: Remove this when issue #4760 is resolved, because at that point
// tests/compute/simple.slang should cover the same issue.
SLANG_UNIT_TEST(cudaCodeGenBug)
{
    // We need the CUDA backend for this test
    if (!SLANG_SUCCEEDED(
            unitTestContext->slangGlobalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            outputBuffer[dispatchThreadID.x] = float(dispatchThreadID.x);
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    ComPtr<slang::IBlob> code;
    auto res = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    SLANG_CHECK(code != nullptr && code->getBufferSize() != 0);
}
