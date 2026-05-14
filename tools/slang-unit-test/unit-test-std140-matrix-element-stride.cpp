// unit-test-std140-matrix-element-stride.cpp

// Regression test for #10961: for a `cbuffer` containing a `float3x3` matrix,
// reflection must report a 16-byte matrix element stride under std140 / SPIR-V,
// not the natural `float3` size of 12.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;

namespace
{
slang::VariableLayoutReflection* findParameterByName(
    slang::ProgramLayout* programLayout,
    char const* name)
{
    for (unsigned i = 0; i < programLayout->getParameterCount(); ++i)
    {
        auto varLayout = programLayout->getParameterByIndex(i);
        auto varName = varLayout ? varLayout->getName() : nullptr;
        if (varName && strcmp(varName, name) == 0)
            return varLayout;
    }
    return nullptr;
}
} // namespace

SLANG_UNIT_TEST(std140Float3x3MatrixElementStrideReflection)
{
    const char* source = R"(
        cbuffer Uniforms
        {
            float3x3 matrix;
        };

        RWStructuredBuffer<float> outputBuffer;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            outputBuffer[0] = matrix[0][0];
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc{};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc{};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "std140_matrix_element_stride",
        "std140-matrix-element-stride.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composite;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            components,
            2,
            composite.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        composite->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(programLayout != nullptr);

    auto uniformsVarLayout = findParameterByName(programLayout, "Uniforms");
    SLANG_CHECK_ABORT(uniformsVarLayout != nullptr);

    auto uniformsTypeLayout = uniformsVarLayout->getTypeLayout();
    SLANG_CHECK_ABORT(uniformsTypeLayout != nullptr);

    auto uniformsElementTypeLayout = uniformsTypeLayout->getElementTypeLayout();
    SLANG_CHECK_ABORT(uniformsElementTypeLayout != nullptr);

    auto matrixFieldIndex = uniformsElementTypeLayout->findFieldIndexByName("matrix");
    SLANG_CHECK_ABORT(matrixFieldIndex >= 0);

    auto matrixVarLayout = uniformsElementTypeLayout->getFieldByIndex((unsigned)matrixFieldIndex);
    SLANG_CHECK_ABORT(matrixVarLayout != nullptr);

    auto matrixTypeLayout = matrixVarLayout->getTypeLayout();
    SLANG_CHECK_ABORT(matrixTypeLayout != nullptr);

    auto matrixElementTypeLayout = matrixTypeLayout->getElementTypeLayout();
    SLANG_CHECK_ABORT(matrixElementTypeLayout != nullptr);

    // std140 requires a 16-byte stride between the rows of a float3x3.
    SLANG_CHECK(matrixElementTypeLayout->getStride() == 16);
}
