// unit-test-entry-point-hash.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <string.h>

using namespace Slang;

// getEntryPointHash is a shader-cache key, so two linked programs whose generated code
// for the requested entry point differs must not share a hash. A sibling entry point
// participates in program layout (an earlier entry point's `ParameterBlock` claims a
// descriptor space and shifts later entry points' sets), and the same program must
// always hash identically, in any process, or a persistent shader cache can never hit.
// Both inputs flow into the hash through each child component's buildHash, which
// CompositeComponentType::buildHash folds in child order.

// The fixture is shared by both test cases and is deliberately shaped: `psA` claims an
// entry-point `ParameterBlock` (which shifts `vsMain`'s descriptor sets), `psB` claims
// nothing, and `vsMain` reads the module-scope `gHeap` so its own layout is
// sibling-sensitive. `psB` doubles as the rename target in the renamed-sibling case.
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

// Treats a missing blob as non-equal, so a missing hash and a hash collision both land
// on the same comparison failure rather than one of them passing silently.
static bool _blobContentEquals(slang::IBlob* left, slang::IBlob* right)
{
    if (!left || !right || left->getBufferSize() != right->getBufferSize())
        return false;
    return ::memcmp(left->getBufferPointer(), right->getBufferPointer(), left->getBufferSize()) ==
           0;
}

// Create a session with a bare SPIR-V target and load kSiblingSource as `moduleName`.
// The tests only link and hash, never compile to a final artifact, so the bare target
// keeps them hermetic — no SPIR-V tooling is needed on the machine.
static SlangResult _createSessionAndModule(
    slang::IGlobalSession* globalSession,
    const char* moduleName,
    ComPtr<slang::ISession>& outSession,
    ComPtr<slang::IModule>& outModule)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, outSession.writeRef()));

    ComPtr<slang::IBlob> diagnostics;
    outModule = outSession->loadModuleFromSourceString(
        moduleName,
        "sibling.slang",
        kSiblingSource,
        diagnostics.writeRef());
    if (!outModule)
    {
        // The shader source is the one input here that can fail to parse; surface the
        // compiler's diagnostics so a red run identifies the module load as the step
        // that broke.
        if (diagnostics)
        {
            fprintf(
                stderr,
                "module '%s' failed to load: %s\n",
                moduleName,
                (const char*)diagnostics->getBufferPointer());
        }
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

// Link {module, fragmentEPName, vsMain} and return vsMain's entry-point hash. Entry
// points are indexed in composite-child order and a module child contributes none, so
// the composite's entry points are [fragmentEP, vsMain] and vsMain is index 1.
static SlangResult _getVsMainHash(
    slang::ISession* session,
    slang::IModule* module,
    const char* fragmentEPName,
    SlangStage fragmentStage,
    ComPtr<slang::IBlob>& outHash)
{
    ComPtr<slang::IBlob> diagnostics;
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

    ComPtr<slang::ISession> session;
    ComPtr<slang::IModule> module;
    SLANG_RETURN_ON_FAIL(_createSessionAndModule(globalSession.get(), "sibling", session, module));

    ComPtr<slang::IBlob> psAHash;
    SLANG_RETURN_ON_FAIL(_getVsMainHash(session, module, "psA", SLANG_STAGE_FRAGMENT, psAHash));

    ComPtr<slang::IBlob> psBHash;
    SLANG_RETURN_ON_FAIL(_getVsMainHash(session, module, "psB", SLANG_STAGE_FRAGMENT, psBHash));

    // psA's entry-point ParameterBlock shifts vsMain's descriptor sets, so vsMain's
    // generated code differs between the two programs and its cache key must differ.
    if (_blobContentEquals(psAHash, psBHash))
        return SLANG_FAIL;

    // The same program must still hash identically within one session, so that fixing
    // the collision above did not buy a spurious cache miss.
    ComPtr<slang::IBlob> psAHashRepeat;
    SLANG_RETURN_ON_FAIL(
        _getVsMainHash(session, module, "psA", SLANG_STAGE_FRAGMENT, psAHashRepeat));

    if (!_blobContentEquals(psAHash, psAHashRepeat))
        return SLANG_FAIL;

    // The same program must also hash identically when built from scratch in a second,
    // identically-configured session. Rebuilding inside the first session would reuse
    // its module cache and objects, so it cannot catch a pointer-valued hash input; a
    // persistent shader cache only produces hits if the key is stable across processes.
    // The module name must match so the mangled names match.
    ComPtr<slang::ISession> session2;
    ComPtr<slang::IModule> module2;
    SLANG_RETURN_ON_FAIL(
        _createSessionAndModule(globalSession.get(), "sibling", session2, module2));

    ComPtr<slang::IBlob> psAHashSession2;
    SLANG_RETURN_ON_FAIL(
        _getVsMainHash(session2, module2, "psA", SLANG_STAGE_FRAGMENT, psAHashSession2));

    if (!_blobContentEquals(psAHash, psAHashSession2))
        return SLANG_FAIL;

    return SLANG_OK;
}

// Link {module, psB, rename(vsMain, "vsX"), rename(baseEPName, siblingOverride)} and
// return the renamed vsMain's entry-point hash. Entry points are indexed in
// composite-child order and the module child contributes none, so the composite's entry
// points are [psB, vsX, renamedSibling] and the renamed vsMain is index 1.
static SlangResult _getRenamedSiblingHash(
    slang::ISession* session,
    slang::IModule* module,
    const char* baseEPName,
    const char* siblingOverride,
    ComPtr<slang::IBlob>& outHash)
{
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IEntryPoint> psBEP, vsMainEP, baseEP;
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "psB", SLANG_STAGE_FRAGMENT, psBEP.writeRef(), diagnostics.writeRef()));
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "vsMain", SLANG_STAGE_VERTEX, vsMainEP.writeRef(), diagnostics.writeRef()));
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        baseEPName, SLANG_STAGE_FRAGMENT, baseEP.writeRef(), diagnostics.writeRef()));

    ComPtr<slang::IComponentType> renamedVsMain, renamedSibling;
    SLANG_RETURN_ON_FAIL(vsMainEP->renameEntryPoint("vsX", renamedVsMain.writeRef()));
    SLANG_RETURN_ON_FAIL(baseEP->renameEntryPoint(siblingOverride, renamedSibling.writeRef()));

    slang::IComponentType* components[] = {
        module,
        psBEP.get(),
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

    ComPtr<slang::ISession> session;
    ComPtr<slang::IModule> module;
    SLANG_RETURN_ON_FAIL(
        _createSessionAndModule(globalSession.get(), "renamed-sibling", session, module));

    // Conservative case: the requested entry point (renamed vsMain) is identical in
    // both programs, which differ only in the *sibling* renamed entry point's override
    // (same underlying psB). A renamed sibling contributes nothing to the requested
    // entry point's own code, so distinguishing these hashes can only turn a cache hit
    // into a miss — never a miss into a false hit.
    ComPtr<slang::IBlob> siblingPsYHash;
    SLANG_RETURN_ON_FAIL(
        _getRenamedSiblingHash(session, module, "psB", "psY", siblingPsYHash));

    ComPtr<slang::IBlob> siblingPsZHash;
    SLANG_RETURN_ON_FAIL(
        _getRenamedSiblingHash(session, module, "psB", "psZ", siblingPsZHash));

    if (_blobContentEquals(siblingPsYHash, siblingPsZHash))
        return SLANG_FAIL;

    ComPtr<slang::IBlob> siblingPsYHashRepeat;
    SLANG_RETURN_ON_FAIL(
        _getRenamedSiblingHash(session, module, "psB", "psY", siblingPsYHashRepeat));

    if (!_blobContentEquals(siblingPsYHash, siblingPsYHashRepeat))
        return SLANG_FAIL;

    // Load-bearing case: two programs whose siblings are *distinct underlying entry
    // points* (psA's ParameterBlock shifts vsMain's descriptor sets; psB's does not)
    // renamed to the same override name. The requested entry point's generated code
    // differs, so the hashes must differ. An empty buildHash on the rename wrapper
    // would drop the delegated base identity and make exactly this pair collide — the
    // false hit the first test case guards against, reintroduced through the wrapper.
    ComPtr<slang::IBlob> renamedPsAHash;
    SLANG_RETURN_ON_FAIL(
        _getRenamedSiblingHash(session, module, "psA", "psQ", renamedPsAHash));

    ComPtr<slang::IBlob> renamedPsBHash;
    SLANG_RETURN_ON_FAIL(
        _getRenamedSiblingHash(session, module, "psB", "psQ", renamedPsBHash));

    if (_blobContentEquals(renamedPsAHash, renamedPsBHash))
        return SLANG_FAIL;

    return SLANG_OK;
}

SLANG_UNIT_TEST(entryPointHash)
{
    SLANG_CHECK(SLANG_SUCCEEDED(_testSiblingEntryPointAffectsHash()));
    SLANG_CHECK(SLANG_SUCCEEDED(_testRenamedSiblingEntryPointAffectsHash()));
}
