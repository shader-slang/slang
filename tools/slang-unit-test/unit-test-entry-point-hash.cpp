// unit-test-entry-point-hash.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// getEntryPointHash is a shader-cache key, so two linked programs whose generated code
// for the requested entry point differs must not share a hash. A sibling entry point
// participates in program layout (an earlier entry point's `ParameterBlock` claims a
// descriptor space and shifts later entry points' sets), and an entry-point name
// override appears in the requested entry point's generated code (e.g. SPIR-V
// OpEntryPoint names the function). Both of those inputs flow into the hash through
// each child component's buildHash, which CompositeComponentType::buildHash folds in
// child order.

static const char* kSiblingSource = R"(
struct Params { Texture2D tex; SamplerState smp; float4 tint; }
struct VsParams { StructuredBuffer<float4> verts; }
StructuredBuffer<float4> gHeap;

[shader("vertex")]
float4 vsMain(uniform ParameterBlock<VsParams> v, uint vid: SV_VertexID) : SV_Position
{
    return v.verts[vid] + gHeap[0];
}

[shader("fragment")]
float4 psA(uniform ParameterBlock<Params> p) : SV_Target
{
    return p.tex.Sample(p.smp, float2(0, 0)) * p.tint;
}

[shader("fragment")]
float4 psB() : SV_Target
{
    return float4(1, 0, 0, 1);
}
)";

static bool _blobContentEquals(ISlangBlob* left, ISlangBlob* right)
{
    if (!left || !right || left->getBufferSize() != right->getBufferSize())
        return false;
    return ::memcmp(left->getBufferPointer(), right->getBufferPointer(), left->getBufferSize()) == 0;
}

// Link {module, fragmentEPName, vsMain} and return vsMain's entry-point hash
// (vsMain is entry point index 1 of the composite).
static SlangResult _getVsMainHash(
    slang::ISession* session,
    slang::IModule* module,
    const char* fragmentEPName,
    SlangStage fragmentStage,
    ComPtr<ISlangBlob>& outHash)
{
    ComPtr<ISlangBlob> diagnostics;
    ComPtr<slang::IEntryPoint> fragmentEP, vsMainEP;
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        fragmentEPName, fragmentStage, fragmentEP.writeRef(), diagnostics.writeRef()));
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "vsMain", SLANG_STAGE_VERTEX, vsMainEP.writeRef(), diagnostics.writeRef()));

    slang::IComponentType* components[] = {module, fragmentEP.get(), vsMainEP.get()};
    ComPtr<slang::IComponentType> compositeProgram;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        compositeProgram.writeRef(),
        diagnostics.writeRef()));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(compositeProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()));

    linkedProgram->getEntryPointHash(1, 0, outHash.writeRef());
    return outHash ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _testSiblingEntryPointAffectsHash()
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = session->loadModuleFromSourceString(
        "sibling", "sibling.slang", kSiblingSource, diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;

    ComPtr<ISlangBlob> psAHash;
    SLANG_RETURN_ON_FAIL(_getVsMainHash(session, module, "psA", SLANG_STAGE_FRAGMENT, psAHash));

    ComPtr<ISlangBlob> psBHash;
    SLANG_RETURN_ON_FAIL(_getVsMainHash(session, module, "psB", SLANG_STAGE_FRAGMENT, psBHash));

    // psA's entry-point ParameterBlock shifts vsMain's descriptor sets, so vsMain's
    // generated code differs between the two programs and its cache key must differ.
    if (_blobContentEquals(psAHash, psBHash))
        return SLANG_FAIL;

    // The same program must still hash identically, so that fixing the collision above
    // did not buy a spurious cache miss.
    ComPtr<ISlangBlob> psAHashRepeat;
    SLANG_RETURN_ON_FAIL(
        _getVsMainHash(session, module, "psA", SLANG_STAGE_FRAGMENT, psAHashRepeat));

    if (!_blobContentEquals(psAHash, psAHashRepeat))
        return SLANG_FAIL;

    return SLANG_OK;
}

// Link {module, psA, rename(vsMain, "vsX"), rename(psB, siblingName)} and return the
// renamed vsMain's entry-point hash (entry point index 1: [psA, vsX, sibling]).
static SlangResult _getRenamedSiblingHash(
    slang::ISession* session,
    slang::IModule* module,
    const char* siblingName,
    ComPtr<ISlangBlob>& outHash)
{
    ComPtr<ISlangBlob> diagnostics;
    ComPtr<slang::IEntryPoint> psAEP, vsMainEP, psBEP;
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "psA", SLANG_STAGE_FRAGMENT, psAEP.writeRef(), diagnostics.writeRef()));
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "vsMain", SLANG_STAGE_VERTEX, vsMainEP.writeRef(), diagnostics.writeRef()));
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "psB", SLANG_STAGE_FRAGMENT, psBEP.writeRef(), diagnostics.writeRef()));

    ComPtr<slang::IComponentType> renamedVsMain, renamedSibling;
    SLANG_RETURN_ON_FAIL(vsMainEP->renameEntryPoint("vsX", renamedVsMain.writeRef()));
    SLANG_RETURN_ON_FAIL(psBEP->renameEntryPoint(siblingName, renamedSibling.writeRef()));

    slang::IComponentType* components[] = {
        module,
        psAEP.get(),
        renamedVsMain.get(),
        renamedSibling.get()};
    ComPtr<slang::IComponentType> compositeProgram;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        compositeProgram.writeRef(),
        diagnostics.writeRef()));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(compositeProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()));

    linkedProgram->getEntryPointHash(1, 0, outHash.writeRef());
    return outHash ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _testRenamedSiblingEntryPointAffectsHash()
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = session->loadModuleFromSourceString(
        "renamed-sibling", "renamed-sibling.slang", kSiblingSource, diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;

    // The requested entry point (renamed vsMain) is identical in both programs, and
    // the two programs differ only in the *sibling* renamed entry point's override.
    // A renamed sibling contributes nothing to the requested entry point's own code,
    // so distinguishing the hashes here is conservative rather than load-bearing:
    // hashing the sibling's identity can only turn a cache hit into a miss, never a
    // miss into a false hit. What the case pins down is that the renamed sibling
    // component itself reaches the composite hash — an empty buildHash there would
    // collapse the wrapper's contribution and silently reintroduce the collision the
    // first case guards against.
    ComPtr<ISlangBlob> siblingPsYHash;
    SLANG_RETURN_ON_FAIL(_getRenamedSiblingHash(session, module, "psY", siblingPsYHash));

    ComPtr<ISlangBlob> siblingPsZHash;
    SLANG_RETURN_ON_FAIL(_getRenamedSiblingHash(session, module, "psZ", siblingPsZHash));

    if (_blobContentEquals(siblingPsYHash, siblingPsZHash))
        return SLANG_FAIL;

    ComPtr<ISlangBlob> siblingPsYHashRepeat;
    SLANG_RETURN_ON_FAIL(_getRenamedSiblingHash(session, module, "psY", siblingPsYHashRepeat));

    if (!_blobContentEquals(siblingPsYHash, siblingPsYHashRepeat))
        return SLANG_FAIL;

    return SLANG_OK;
}

SLANG_UNIT_TEST(entryPointHash)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_testSiblingEntryPointAffectsHash()));
    SLANG_CHECK(SLANG_SUCCEEDED(_testRenamedSiblingEntryPointAffectsHash()));
}
