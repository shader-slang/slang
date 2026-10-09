// unit-test-global-type-param-unsized-array.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// shader-slang/slang#13530: the implicit constant buffers `GlobalParams` and `EntryPointParams`
// cannot hold an unsized array of ordinary data. Semantic checking cannot decide whether
// `uniform TT values[]` holds one before a type is bound to `type_param TT`, so
// `ComponentType::specialize` checks the global and entry-point uniforms again. Binding an
// ordinary-data type is diagnosed (E31215); binding a texture type stays valid, because resource
// legalization turns the array into a descriptor array.

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

enum class Composition
{
    ModuleAndEntryPoint,
    ModuleOnly,
    ModuleTwiceAndEntryPoint,
};

static ComPtr<slang::IComponentType> specializeGlobalTypeParams(
    slang::ISession* session,
    Composition composition,
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

    ComPtr<slang::IComponentType> program;
    SlangInt specArgCount = 2;
    switch (composition)
    {
    case Composition::ModuleAndEntryPoint:
        {
            slang::IComponentType* components[] = {module, entryPoint};
            session->createCompositeComponentType(components, 2, program.writeRef());
            break;
        }
    case Composition::ModuleOnly:
        program = module;
        break;
    case Composition::ModuleTwiceAndEntryPoint:
        {
            slang::IComponentType* components[] = {module, module, entryPoint};
            session->createCompositeComponentType(components, 3, program.writeRef());
            specArgCount = 4;
            break;
        }
    }
    if (!program)
        return nullptr;

    // We pass the types as expressions: reflection would lay out the unspecialized program,
    // and `TT globalValues[]` has no layout until `TT` is bound.
    slang::SpecializationArg specArgs[] = {
        slang::SpecializationArg::fromExpr(elementTypeArg),
        slang::SpecializationArg::fromExpr(arrayTypeArg),
        slang::SpecializationArg::fromExpr(elementTypeArg),
        slang::SpecializationArg::fromExpr(arrayTypeArg)};
    ComPtr<slang::IComponentType> specialized;
    program->specialize(specArgs, specArgCount, specialized.writeRef(), outDiagnostics.writeRef());
    return specialized;
}

// A `type_param` declared in one module and used in another is bound when the composite
// `{library, user, entryPoint}` is specialized.
static const char* kLibrarySource = R"(
    module gtplibrary;

    public struct Data
    {
        public float4 v;
    }

    public type_param TT;
    )";

static const char* kUserSource = R"(
    import gtplibrary;

    uniform TT userValues[];
    RWStructuredBuffer<float4> output;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uniform TT userEntryPointValues[])
    {
        output[0] = 1;
    }
    )";

static ComPtr<slang::IComponentType> specializeImportedTypeParam(
    slang::ISession* session,
    const char* typeArg,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    ComPtr<slang::IBlob> diagnosticBlob;
    auto library = session->loadModuleFromSourceString(
        "gtplibrary",
        "gtplibrary.slang",
        kLibrarySource,
        diagnosticBlob.writeRef());
    auto user = session->loadModuleFromSourceString(
        "gtpuser",
        "gtpuser.slang",
        kUserSource,
        diagnosticBlob.writeRef());
    if (!library || !user)
        return nullptr;

    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(user->findEntryPointByName("computeMain", entryPoint.writeRef())))
        return nullptr;

    slang::IComponentType* components[] = {library, user, entryPoint};
    ComPtr<slang::IComponentType> program;
    if (SLANG_FAILED(session->createCompositeComponentType(components, 3, program.writeRef())))
        return nullptr;

    slang::SpecializationArg specArgs[] = {slang::SpecializationArg::fromExpr(typeArg)};
    ComPtr<slang::IComponentType> specialized;
    program->specialize(specArgs, 1, specialized.writeRef(), outDiagnostics.writeRef());
    return specialized;
}

// One generic entry point composed twice, as `{module, entryPoint, entryPoint}`, is specialized
// once per instance.
static const char* kGenericEntryPointSource = R"(
    struct Data
    {
        float4 v;
    }

    RWStructuredBuffer<float4> output;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain<T>(uniform T genericValues[])
    {
        output[0] = 1;
    }
    )";

static ComPtr<slang::IComponentType> specializeGenericEntryPointTwice(
    slang::ISession* session,
    const char* firstTypeArg,
    const char* secondTypeArg,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "gep",
        "gep.slang",
        kGenericEntryPointSource,
        diagnosticBlob.writeRef());
    if (!module)
        return nullptr;

    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
        return nullptr;

    slang::IComponentType* components[] = {module, entryPoint, entryPoint};
    ComPtr<slang::IComponentType> program;
    if (SLANG_FAILED(session->createCompositeComponentType(components, 3, program.writeRef())))
        return nullptr;

    slang::SpecializationArg specArgs[] = {
        slang::SpecializationArg::fromExpr(firstTypeArg),
        slang::SpecializationArg::fromExpr(secondTypeArg)};
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
    // ordinary data once `TT := Data` and `TA := float4[]` are bound, and each is reported once
    // however the module and entry point are composed.
    for (auto composition :
         {Composition::ModuleAndEntryPoint,
          Composition::ModuleOnly,
          Composition::ModuleTwiceAndEntryPoint})
    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized =
            specializeGlobalTypeParams(session, composition, "Data", "float4[]", diagnostics);
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
            Composition::ModuleAndEntryPoint,
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

    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized = specializeImportedTypeParam(session, "Data", diagnostics);
        SLANG_CHECK(specialized == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        auto text = (const char*)diagnostics->getBufferPointer();
        SLANG_CHECK(countOccurrences(text, "error[E31215]") == 2);
        SLANG_CHECK(strstr(text, "userValues") != nullptr);
        SLANG_CHECK(strstr(text, "userEntryPointValues") != nullptr);
    }

    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized = specializeImportedTypeParam(session, "Texture2D<float4>", diagnostics);
        SLANG_CHECK(specialized != nullptr);
    }

    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto specialized =
            specializeGenericEntryPointTwice(session, "Texture2D<float4>", "Data", diagnostics);
        SLANG_CHECK(specialized == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        auto text = (const char*)diagnostics->getBufferPointer();
        SLANG_CHECK(countOccurrences(text, "error[E31215]") == 1);
        SLANG_CHECK(strstr(text, "genericValues") != nullptr);
    }
}
