// unit-test-translation-unit-import.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that the IModule::findAndCheckEntryPoint API supports discovering
// entrypoints without a [shader] attribute.

// Cross-platform environment variable helpers
#ifdef _WIN32
#define SET_SPIRV_VALIDATION() _putenv("SLANG_RUN_SPIRV_VALIDATION=1")
#define UNSET_SPIRV_VALIDATION() _putenv("SLANG_RUN_SPIRV_VALIDATION=")
#else
#define SET_SPIRV_VALIDATION() setenv("SLANG_RUN_SPIRV_VALIDATION", "1", 1)
#define UNSET_SPIRV_VALIDATION() unsetenv("SLANG_RUN_SPIRV_VALIDATION")
#endif

#define DUMP_IR_FOR_DEBUG 0

SLANG_UNIT_TEST(findAndCheckEntryPoint)
{
    // Enable SPIR-V validation
    SET_SPIRV_VALIDATION();

    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        struct BokehSplat
        {
            uint2 color;
        };

        struct DoFSplatParams
        {
            float bokehArea;
            float anisotropy;
            uint pad0;
            uint pad1;
            RWStructuredBuffer<BokehSplat> BokehBuffer;
        };
        ParameterBlock<DoFSplatParams> gDoFSplatParams;

        // Test absence of attribute [shader("...")] intentionally
        // [shader("fragment")]
        float4 fragMain(float4 pos:SV_Position, uint instanceIndex: SV_InstanceID) : SV_Target
        {
            BokehSplat bokeh = gDoFSplatParams.BokehBuffer[instanceIndex];
            return float4(bokeh.color, 0, 1);
        }


        interface I { int getValue(); }
        struct X : I { int getValue() { return 100; } }

        float4 vertMain<T:I, int n>(uniform T o) : SV_Position {
            return float4(o.getValue(), n, 0, 1);
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
#if DUMP_IR_FOR_DEBUG
    slang::CompilerOptionEntry compilerOptionEntry = {};
    compilerOptionEntry.name = slang::CompilerOptionName::DumpIr;
    compilerOptionEntry.value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptionEntry.value.intValue0 = 1;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &compilerOptionEntry;
#endif
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
        "vertMain",
        SLANG_STAGE_VERTEX,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    // Build specialization arguments
    slang::SpecializationArg args[] = {
        // T = X (a type that implements interface I)
        slang::SpecializationArg::fromType(module->getLayout()->findTypeByName("X")),
        // n = 8 (an integer constant)
        slang::SpecializationArg::fromExpr("8"),
    };

    ComPtr<slang::IComponentType> specializedEntryPoint;
    entryPoint->specialize(args, 2, specializedEntryPoint.writeRef(), nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, specializedEntryPoint.get()};
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

    // Check for validation errors
    if (diagnosticBlob)
    {
        const char* diagText = (const char*)diagnosticBlob->getBufferPointer();
        if (strstr(diagText, "Validation of generated SPIR-V failed"))
        {
            SLANG_CHECK(false); // Fail the test on validation error
        }
#if DUMP_IR_FOR_DEBUG
        // Save the diagnostic blob to a file
        FILE* f = fopen("diagnostic.txt", "wb");
        fwrite(diagText, 1, strlen(diagText), f);
        fclose(f);
        printf("diagnostic.txt created\n");
#endif
    }

    SLANG_CHECK(code != nullptr);
    SLANG_CHECK(code->getBufferSize() != 0);

#if DUMP_IR_FOR_DEBUG
    FILE* f = fopen("check-entrypoint.spv", "wb");
    fwrite(code->getBufferPointer(), 1, code->getBufferSize(), f);
    fclose(f);
    printf("check-entrypoint.spv created\n");
#endif
    // Restore environment variable to not affect other tests
    UNSET_SPIRV_VALIDATION();
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