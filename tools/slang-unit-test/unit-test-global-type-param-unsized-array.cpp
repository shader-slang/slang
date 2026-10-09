// unit-test-global-type-param-unsized-array.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// shader-slang/slang#13530: a global uniform `TT values[]` goes into the implicit constant
// buffer, which cannot hold an unsized array of ordinary data. Semantic checking cannot decide
// that before a type is bound to `type_param TT`, so specialization checks it again. Binding an
// ordinary-data type is diagnosed (E31215); binding a texture type stays valid, because
// resource legalization turns the array into a descriptor array.

static const char* kGlobalTypeParamUnsizedArraySource = R"(
    struct Data
    {
        float4 v;
    }

    type_param TT;

    uniform TT values[];
    RWStructuredBuffer<float4> output;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain()
    {
        output[0] = 1;
    }
    )";

// Specialize `type_param TT` with `typeArg` and return the result of `specialize`, writing any
// diagnostics to `outDiagnostics`. Returns null when the module or entry point is not found.
static ComPtr<slang::IComponentType> specializeGlobalTypeParam(
    slang::ISession* session,
    const char* typeArg,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        kGlobalTypeParamUnsizedArraySource,
        diagnosticBlob.writeRef());
    if (!module)
        return nullptr;

    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
        return nullptr;

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    if (SLANG_FAILED(session->createCompositeComponentType(components, 2, program.writeRef())))
        return nullptr;

    // We pass the type as an expression rather than reflecting it, because laying out the
    // unspecialized program would need a layout for the unsized `TT values[]`.
    slang::SpecializationArg specArgs[] = {slang::SpecializationArg::fromExpr(typeArg)};
    ComPtr<slang::IComponentType> specialized;
    program->specialize(specArgs, 1, specialized.writeRef(), outDiagnostics.writeRef());
    return specialized;
}

SLANG_UNIT_TEST(globalTypeParamUnsizedArray)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // An unsized array of ordinary data is diagnosed when `TT` is bound.
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized = specializeGlobalTypeParam(session, "Data", diagnostics);
        SLANG_CHECK(specialized == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(strstr((const char*)diagnostics->getBufferPointer(), "E31215") != nullptr);
    }

    // An unsized array of textures stays valid and produces code.
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized = specializeGlobalTypeParam(session, "Texture2D<float4>", diagnostics);
        SLANG_CHECK_ABORT(specialized != nullptr);

        ComPtr<slang::IComponentType> linkedProgram;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(specialized->link(linkedProgram.writeRef())));
        ComPtr<slang::IBlob> code;
        diagnostics = nullptr;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(code != nullptr && code->getBufferSize() != 0);
    }
}
