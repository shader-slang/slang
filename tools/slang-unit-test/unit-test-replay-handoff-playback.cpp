// unit-test-replay-handoff.cpp
// Sharpens issue #10479 review hand-off points beyond roundtrip coverage.

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <cstring>
#include <vector>

// Default iteration count is CI-friendly. Crank up locally for actual stress testing.
// Each iteration creates a fresh GlobalSession (with embedded core module) which is a
// heavy allocation; high counts can OOM memory-constrained machines. Override via
// -DSLANG_UNIT_TEST_REPLAY_RELEASE_STRESS_ITERS=N at configure time.
#ifndef SLANG_UNIT_TEST_REPLAY_RELEASE_STRESS_ITERS
#define SLANG_UNIT_TEST_REPLAY_RELEASE_STRESS_ITERS 10
#endif

// Verify a handle is in the handle table and that the table is a true bijection:
// h -> proxy must resolve to a proxy, and proxy -> h must produce the original handle.
static void assertHandleRoundTrips(ReplayContext& r, uint64_t h)
{
    if (h == kNullHandle)
        return;
    ISlangUnknown* p = r.getProxy(h);
    SLANG_CHECK(p != nullptr);
    SLANG_CHECK(r.getProxyHandle(p) == h);
}

// =============================================================================
// Full pipeline: record a typical Slang compilation, capture every produced
// proxy handle, replay the whole stream, then verify each captured handle
// still round-trips through the rebuilt handle table.
// =============================================================================
SLANG_UNIT_TEST(replayContextOutputHandleValueEqualityAcrossPlayback)
{
    REPLAY_TEST;

    // --- Record phase: drive a full compilation pipeline ---
    ctx().setMode(Mode::Record);

    // Create a global session and pick a SPIR-V profile.
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    // Create a session bound to that single target.
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    // Load a minimal compute shader from a source string. Each of the
    // remaining steps below produces another proxy whose handle we'll
    // capture before tearing down.
    const char* shaderCode = "[shader(\"compute\")]\n"
                             "[numthreads(1,1,1)]\n"
                             "void computeMain() {}\n";

    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = session->loadModuleFromSourceString(
        "test-module",
        "test-module.slang",
        shaderCode,
        diagnosticsBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    // Resolve the entry point, then compose+link to form a final program.
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(
        SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", entryPoint.writeRef())));

    slang::IComponentType* components[] = {module, entryPoint.get()};
    Slang::ComPtr<slang::IComponentType> composite;
    SLANG_CHECK(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        2,
        composite.writeRef(),
        diagnosticsBlob.writeRef())));

    Slang::ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(SLANG_SUCCEEDED(composite->link(linked.writeRef(), diagnosticsBlob.writeRef())));

    // Snapshot every interesting handle assigned during recording. Each one
    // must survive the replay and still resolve to a live proxy afterward.
    uint64_t hGlobal = ctx().getProxyHandle(globalSession.get());
    uint64_t hSession = ctx().getProxyHandle(session.get());
    uint64_t hModule = ctx().getProxyHandle(module);
    uint64_t hEntry = ctx().getProxyHandle(entryPoint.get());
    uint64_t hComposite = ctx().getProxyHandle(composite.get());
    uint64_t hLinked = ctx().getProxyHandle(linked.get());

    // --- Playback phase ---
    // executeAll replays the full pipeline: every proxy that was registered
    // during recording is registered again here, and Fix D's keep-alive ref
    // keeps them all alive past the replayed release() calls.
    ctx().switchToPlayback();
    ctx().executeAll();
    ctx().disable();

    // --- Verify: every recorded handle still maps to a live proxy ---
    assertHandleRoundTrips(ctx(), hGlobal);
    assertHandleRoundTrips(ctx(), hSession);
    assertHandleRoundTrips(ctx(), hModule);
    assertHandleRoundTrips(ctx(), hEntry);
    assertHandleRoundTrips(ctx(), hComposite);
    assertHandleRoundTrips(ctx(), hLinked);
}
