// unit-test-c-data-layout-stride.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test that ConstantBuffer<T, CDataLayout> reflection AND generated SPIRV
// both use C layout rules, even when the session has forceScalarLayout enabled.
//
// ContiguousBufferView = { Ptr<float> data; uint strides[1]; }
//   -> ptr at 0 (8B, align 8), strides at 8 (4B) => raw 12, align 8, C padded = 16
//
// ReductionParams = { ContiguousBufferView input; LastDimLayout layout; uint numGroups; Ptr<float>
// stats; }
//   -> input at 0 (16B), layout at 16 (8B), numGroups at 24 (4B), stats at 32 (8B)
//   => total = 40, align = 8
//
// Previously, forceScalarLayout would override the explicit CDataLayout annotation,
// causing ContiguousBufferView to not be padded (size 12 instead of 16), which shifted
// all subsequent field offsets and produced a total size of 32 instead of 40.

static void _testCDataLayoutReflectionStride(UnitTestContext* context, bool forceScalarLayout)
{
    const char* userSourceBody = R"(
        struct ContiguousBufferView
        {
            Ptr<float> data;
            uint strides[1];
        };

        struct LastDimLayout
        {
            uint numRows;
            uint numCols;
        };

        struct ReductionParams
        {
            ContiguousBufferView input;
            LastDimLayout layout;
            uint numGroups;
            Ptr<float> stats;
        };

        RWStructuredBuffer<float> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(ConstantBuffer<ReductionParams, CDataLayout> params)
        {
            outputBuffer[0] = params.numGroups;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    targetDesc.forceGLSLScalarBufferLayout = forceScalarLayout;

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

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    // =========================================================================
    // 1. Check reflection API stride
    // =========================================================================
    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(programLayout != nullptr);

    SLANG_CHECK_ABORT(programLayout->getEntryPointCount() == 1);
    auto entryPointLayout = programLayout->getEntryPointByIndex(0);
    SLANG_CHECK_ABORT(entryPointLayout != nullptr);

    slang::VariableLayoutReflection* paramsVar = nullptr;
    for (unsigned i = 0; i < entryPointLayout->getParameterCount(); i++)
    {
        auto var = entryPointLayout->getParameterByIndex(i);
        if (var && strcmp(var->getName(), "params") == 0)
        {
            paramsVar = var;
            break;
        }
    }
    SLANG_CHECK_ABORT(paramsVar != nullptr);

    auto cbufferTypeLayout = paramsVar->getTypeLayout();
    SLANG_CHECK_ABORT(cbufferTypeLayout != nullptr);

    auto elementTypeLayout = cbufferTypeLayout->getElementTypeLayout();
    SLANG_CHECK_ABORT(elementTypeLayout != nullptr);

    size_t stride = elementTypeLayout->getStride();
    SLANG_CHECK(stride == 40);

    // =========================================================================
    // 2. Check generated SPIRV-ASM for correct OpMemberDecorate offsets
    // =========================================================================
    // Compile to SPIRV-ASM so we can inspect the text output.
    slang::TargetDesc spirvAsmTargetDesc = {};
    spirvAsmTargetDesc.format = SLANG_SPIRV_ASM;
    spirvAsmTargetDesc.profile = globalSession->findProfile("spirv_1_5");
    spirvAsmTargetDesc.forceGLSLScalarBufferLayout = forceScalarLayout;

    slang::SessionDesc spirvAsmSessionDesc = {};
    spirvAsmSessionDesc.targetCount = 1;
    spirvAsmSessionDesc.targets = &spirvAsmTargetDesc;

    ComPtr<slang::ISession> spirvAsmSession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(spirvAsmSessionDesc, spirvAsmSession.writeRef()) == SLANG_OK);

    auto spirvAsmModule = spirvAsmSession->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(spirvAsmModule != nullptr);

    ComPtr<slang::IEntryPoint> spirvAsmEntryPoint;
    spirvAsmModule->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        spirvAsmEntryPoint.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(spirvAsmEntryPoint != nullptr);

    ComPtr<slang::IComponentType> spirvAsmComposite;
    slang::IComponentType* spirvAsmComponents[] = {spirvAsmModule, spirvAsmEntryPoint.get()};
    spirvAsmSession->createCompositeComponentType(
        spirvAsmComponents,
        2,
        spirvAsmComposite.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(spirvAsmComposite != nullptr);

    ComPtr<slang::IComponentType> spirvAsmLinked;
    spirvAsmComposite->link(spirvAsmLinked.writeRef(), nullptr);
    SLANG_CHECK_ABORT(spirvAsmLinked != nullptr);

    ComPtr<slang::IBlob> spirvAsmBlob;
    spirvAsmLinked->getTargetCode(0, spirvAsmBlob.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(spirvAsmBlob != nullptr);

    const char* spirvAsm = (const char*)spirvAsmBlob->getBufferPointer();
    size_t spirvAsmSize = spirvAsmBlob->getBufferSize();
    String spirvAsmStr(spirvAsm, spirvAsm + spirvAsmSize);

    // ReductionParams_c member 1 (layout) must be at Offset 16, not 12.
    // This confirms ContiguousBufferView was padded from 12 to 16.
    bool hasLayoutAt16 =
        spirvAsmStr.indexOf("OpMemberDecorate %ReductionParams_c 1 Offset 16") != -1;
    // ReductionParams_c member 3 (stats) must be at Offset 32.
    bool hasStatsAt32 =
        spirvAsmStr.indexOf("OpMemberDecorate %ReductionParams_c 3 Offset 32") != -1;

    SLANG_CHECK(hasLayoutAt16);
    SLANG_CHECK(hasStatsAt32);
}

SLANG_UNIT_TEST(CDataLayoutReflectionStride)
{
    _testCDataLayoutReflectionStride(unitTestContext, false);
}

SLANG_UNIT_TEST(CDataLayoutReflectionStrideWithScalarLayout)
{
    _testCDataLayoutReflectionStride(unitTestContext, true);
}
