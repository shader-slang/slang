// unit-test-matrix-layout-option-override.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// returns whether the matrix field compiles row-major, with `option` set on the target or program
static bool compilesRowMajor(
    UnitTestContext* unitTestContext,
    SlangMatrixLayoutMode sessionLayout,
    slang::CompilerOptionEntry option,
    bool onProgram)
{
    const char* source = R"(
        struct Params { float2x3 m; }
        ConstantBuffer<Params> params;
        RWStructuredBuffer<float3> outputBuffer;

        [numthreads(1, 1, 1)]
        void computeMain()
        {
            outputBuffer[0] = params.m[1];
        }
    )";

    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    if (!onProgram)
    {
        targetDesc.compilerOptionEntryCount = 1;
        targetDesc.compilerOptionEntries = &option;
    }

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.defaultMatrixLayoutMode = sessionLayout;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = session->loadModuleFromSourceString(
        "matrixLayoutOptionOverride",
        "matrix-layout-option-override.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        composite->linkWithOptions(
            program.writeRef(),
            onProgram ? 1 : 0,
            &option,
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> code;
    SLANG_CHECK_ABORT(
        program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    // Slang row-major is SPIR-V ColMajor
    return strstr((const char*)code->getBufferPointer(), " ColMajor") != nullptr;
}

SLANG_UNIT_TEST(matrixLayoutOptionOverridesSession)
{
    const auto row = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    const auto column = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    const auto rowFlag = slang::CompilerOptionName::MatrixLayoutRow;
    const auto columnFlag = slang::CompilerOptionName::MatrixLayoutColumn;

    struct TestCase
    {
        SlangMatrixLayoutMode sessionLayout;
        slang::CompilerOptionName flag;
        int value;
        bool onProgram;
        bool expectRowMajor;
    };
    const TestCase cases[] = {
        {row, columnFlag, 1, false, false},
        {column, rowFlag, 1, false, true},
        {row, columnFlag, 1, true, false},
        {column, rowFlag, 1, true, true},
        // a false flag sets no layout, so the session's layout stays
        {column, rowFlag, 0, false, false},
    };

    for (const auto& testCase : cases)
    {
        slang::CompilerOptionEntry option = {};
        option.name = testCase.flag;
        option.value.kind = slang::CompilerOptionValueKind::Int;
        option.value.intValue0 = testCase.value;
        SLANG_CHECK(
            compilesRowMajor(unitTestContext, testCase.sessionLayout, option, testCase.onProgram) ==
            testCase.expectRowMajor);
    }
}
