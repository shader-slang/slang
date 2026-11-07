// unit-test-matrix-layout-option-override.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

struct MatrixLayoutResult
{
    bool codegenRowMajor;
    bool reflectionRowMajor;
};

// compiles a matrix field with the given options on the target and program, a count of 0 sets none
static MatrixLayoutResult compileMatrixLayout(
    UnitTestContext* unitTestContext,
    SlangMatrixLayoutMode sessionLayout,
    int targetOptionCount,
    slang::CompilerOptionEntry targetOption,
    int programOptionCount,
    slang::CompilerOptionEntry programOption)
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
    targetDesc.compilerOptionEntryCount = targetOptionCount;
    targetDesc.compilerOptionEntries = &targetOption;

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
            programOptionCount,
            &programOption,
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> code;
    SLANG_CHECK_ABORT(
        program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    slang::ProgramLayout* layout = program->getLayout(0);
    SLANG_CHECK_ABORT(layout != nullptr);
    slang::TypeLayoutReflection* matrixLayout = layout->getParameterByIndex(0)
                                                    ->getTypeLayout()
                                                    ->getElementTypeLayout()
                                                    ->getFieldByIndex(0)
                                                    ->getTypeLayout();

    MatrixLayoutResult result;
    // Slang row-major is SPIR-V ColMajor
    result.codegenRowMajor = strstr((const char*)code->getBufferPointer(), " ColMajor") != nullptr;
    result.reflectionRowMajor =
        matrixLayout->getMatrixLayoutMode() == SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    return result;
}

SLANG_UNIT_TEST(matrixLayoutOptionOverridesSession)
{
    const auto row = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    const auto column = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    const auto rowFlag = slang::CompilerOptionName::MatrixLayoutRow;
    const auto columnFlag = slang::CompilerOptionName::MatrixLayoutColumn;

    struct LayoutFlag
    {
        bool isSet;
        slang::CompilerOptionName name;
        int value;
    };
    const LayoutFlag none = {false, rowFlag, 0};
    const LayoutFlag setRow = {true, rowFlag, 1};
    const LayoutFlag setColumn = {true, columnFlag, 1};
    // a false flag sets no layout, so the layout below it stays
    const LayoutFlag clearRow = {true, rowFlag, 0};

    struct TestCase
    {
        SlangMatrixLayoutMode sessionLayout;
        LayoutFlag target;
        LayoutFlag program;
        bool expectRowMajor;
    };
    const TestCase cases[] = {
        {row, setColumn, none, false},
        {column, setRow, none, true},
        {row, none, setColumn, false},
        {column, none, setRow, true},
        {column, clearRow, none, false},
        {row, setRow, setColumn, false},
        {column, setColumn, setRow, true},
    };

    for (const auto& testCase : cases)
    {
        auto makeEntry = [](LayoutFlag flag)
        {
            slang::CompilerOptionEntry entry = {};
            entry.name = flag.name;
            entry.value.kind = slang::CompilerOptionValueKind::Int;
            entry.value.intValue0 = flag.value;
            return entry;
        };
        MatrixLayoutResult result = compileMatrixLayout(
            unitTestContext,
            testCase.sessionLayout,
            testCase.target.isSet ? 1 : 0,
            makeEntry(testCase.target),
            testCase.program.isSet ? 1 : 0,
            makeEntry(testCase.program));
        SLANG_CHECK(result.codegenRowMajor == testCase.expectRowMajor);
        SLANG_CHECK(result.reflectionRowMajor == testCase.expectRowMajor);
    }
}
