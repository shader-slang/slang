// unit-test-metal-packed-buffer-stride.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// On Metal, reflection reports a structured buffer's element stride according
// to the layout the generated MSL actually uses: natural (scalar-aligned,
// tightly packed) layout, so a `float3` element has stride 12, matching the
// `packed_float3` storage `MetalBufferElementTypeLoweringPolicy` emits.
static size_t _getMetalStructuredBufferFloat3Stride(bool forceScalarLayout)
{
    const char* userSourceBody = R"(
        StructuredBuffer<float3> positions;
        RWStructuredBuffer<float> output;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            output[tid.x] = positions[tid.x].x;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_METAL;
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

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(programLayout != nullptr);

    slang::VariableLayoutReflection* positionsVar = nullptr;
    for (unsigned i = 0; i < programLayout->getParameterCount(); i++)
    {
        auto var = programLayout->getParameterByIndex(i);
        if (var && var->getName() && strcmp(var->getName(), "positions") == 0)
        {
            positionsVar = var;
            break;
        }
    }
    SLANG_CHECK_ABORT(positionsVar != nullptr);

    auto elementTypeLayout = positionsVar->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(elementTypeLayout != nullptr);
    return elementTypeLayout->getStride();
}

SLANG_UNIT_TEST(metalPackedBufferStride)
{
    // Packed (12-byte) stride is the default and is unaffected by the flag.
    SLANG_CHECK(_getMetalStructuredBufferFloat3Stride(false) == 12);
    SLANG_CHECK(_getMetalStructuredBufferFloat3Stride(true) == 12);
}
