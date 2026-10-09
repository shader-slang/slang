// unit-test-global-type-param-unsized-array.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// shader-slang/slang#13530: the implicit constant buffers `GlobalParams` and `EntryPointParams`
// cannot hold an unsized array of ordinary data. Semantic checking cannot decide whether
// `uniform TT values[]` holds one before a type is bound to `type_param TT`, so specialization
// checks the global and entry-point uniforms again. Binding an ordinary-data type is diagnosed
// (E31215); binding a texture type stays valid, because resource legalization turns the array
// into a descriptor array.

static const char* kGlobalTypeParamUnsizedArraySource = R"(
    struct Data
    {
        float4 v;
    }

    type_param TT;
    type_param TA;

    uniform TT globalValues[];
    uniform TA globalArray;
    RWStructuredBuffer<float4> output;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uniform TT entryPointValues[])
    {
        output[0] = 1;
    }
    )";

static ComPtr<slang::IComponentType> specializeGlobalTypeParams(
    slang::ISession* session,
    const char* elementTypeArg,
    const char* arrayTypeArg,
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

    // We pass the types as expressions: reflection would lay out the unspecialized program,
    // and `TT globalValues[]` has no layout until `TT` is bound.
    slang::SpecializationArg specArgs[] = {
        slang::SpecializationArg::fromExpr(elementTypeArg),
        slang::SpecializationArg::fromExpr(arrayTypeArg)};
    ComPtr<slang::IComponentType> specialized;
    program->specialize(specArgs, 2, specialized.writeRef(), outDiagnostics.writeRef());
    return specialized;
}

static int countOccurrences(const char* text, const char* pattern)
{
    int count = 0;
    for (auto found = strstr(text, pattern); found; found = strstr(found + 1, pattern))
        count++;
    return count;
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

    // `globalValues`, `entryPointValues` and `globalArray` each end in an unsized array of
    // ordinary data once `TT := Data` and `TA := float4[]` are bound.
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized = specializeGlobalTypeParams(session, "Data", "float4[]", diagnostics);
        SLANG_CHECK(specialized == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        auto text = (const char*)diagnostics->getBufferPointer();
        SLANG_CHECK(countOccurrences(text, "error[E31215]") == 3);
        SLANG_CHECK(strstr(text, "globalValues") != nullptr);
        SLANG_CHECK(strstr(text, "entryPointValues") != nullptr);
        SLANG_CHECK(strstr(text, "globalArray") != nullptr);
    }

    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized = specializeGlobalTypeParams(
            session,
            "Texture2D<float4>",
            "Texture2D<float4>[]",
            diagnostics);
        SLANG_CHECK_ABORT(specialized != nullptr);

        ComPtr<slang::IComponentType> linkedProgram;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(specialized->link(linkedProgram.writeRef())));
        ComPtr<slang::IBlob> code;
        diagnostics = nullptr;
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        SLANG_CHECK(code != nullptr && code->getBufferSize() != 0);
    }
}
