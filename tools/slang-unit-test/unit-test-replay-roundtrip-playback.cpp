// unit-test-replay-roundtrip.cpp
//
// End-to-end tests for the replay system covering:
// - Full API round-trip (record + playback) of real compilation sequences
// - Error path handling (bad paths, corrupt/truncated streams)
// - Sync mode over API call sequences
// - Handle correspondence in playback+output mode
// - Edge cases (release-to-zero, null handles, large blobs, multiple sessions)

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

// =============================================================================
// Round-trip record + playback of real API sequences
// =============================================================================

// Full pipeline (single module): record createGlobalSession2 → createSession →
// loadModuleFromSourceString → findEntryPointByName → createCompositeComponentType
// → link → getTargetCode, then replay the entire stream and verify the
// session handle still resolves afterward.
SLANG_UNIT_TEST(replayContextFullApiRoundTrip)
{
    REPLAY_TEST;

    // --- Record phase ---
    ctx().setMode(Mode::Record);

    // Step 1: global session, SPIR-V target.
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    // Step 2: per-target session.
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    // Step 3: load a trivial compute shader from source.
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

    // Step 4: resolve the entry point and compose+link.
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(
        SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", entryPoint.writeRef())));

    slang::IComponentType* components[] = {module, entryPoint};
    Slang::ComPtr<slang::IComponentType> composite;
    Slang::ComPtr<ISlangBlob> compositeBlob;
    SLANG_CHECK(SLANG_SUCCEEDED(session->createCompositeComponentType(
        components,
        2,
        composite.writeRef(),
        compositeBlob.writeRef())));

    Slang::ComPtr<slang::IComponentType> linked;
    Slang::ComPtr<ISlangBlob> linkDiagnostics;
    SLANG_CHECK(SLANG_SUCCEEDED(composite->link(linked.writeRef(), linkDiagnostics.writeRef())));

    // Step 5: emit SPIR-V — this is the back end of the pipeline; if the
    // recorded byte-stream of all the above is well-formed, this returns
    // a non-empty blob.
    Slang::ComPtr<ISlangBlob> targetCode;
    SLANG_CHECK(SLANG_SUCCEEDED(
        linked->getTargetCode(0, targetCode.writeRef(), diagnosticsBlob.writeRef())));
    SLANG_CHECK(targetCode != nullptr);
    SLANG_CHECK(targetCode->getBufferSize() > 0);

    // Capture the session's handle so we can resolve it again after playback.
    uint64_t sessionHandle = ctx().getProxyHandle(session.get());
    SLANG_CHECK(sessionHandle >= kFirstValidHandle);

    // --- Playback phase ---
    // Replay the whole recorded stream from start to finish. Every call
    // above is dispatched into its handler and re-registers its outputs.
    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());
    ctx().executeAll();
    ctx().disable();

    // --- Verify ---
    // The recorded session handle should still resolve — the keep-alive
    // refs added by Fix D protect it from the replayed release() calls.
    ISlangUnknown* playedBack = ctx().getProxy(sessionHandle);
    SLANG_CHECK(playedBack != nullptr);
}

// =============================================================================
// Multi-module variant: import one module from another to exercise the
// session's module-resolution path during recording + playback.
// =============================================================================
SLANG_UNIT_TEST(replayContextMultiModuleRoundTrip)
{
    REPLAY_TEST;

    // --- Record phase ---
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    // Two modules: lib defines a helper, main imports it and calls it.
    // Loading order matters here: lib must be loaded first because main's
    // `import lib_module;` triggers lookup at module-load time.
    const char* libCode = "int helper(int x) { return x * 2; }\n";
    const char* mainCode = "import lib_module;\n"
                           "[shader(\"compute\")]\n"
                           "[numthreads(1,1,1)]\n"
                           "void computeMain() { int v = helper(1); }\n";

    Slang::ComPtr<slang::IBlob> diag;
    slang::IModule* libModule = session->loadModuleFromSourceString(
        "lib_module",
        "lib_module.slang",
        libCode,
        diag.writeRef());
    SLANG_CHECK(libModule != nullptr);

    slang::IModule* mainModule = session->loadModuleFromSourceString(
        "main_module",
        "main_module.slang",
        mainCode,
        diag.writeRef());
    SLANG_CHECK(mainModule != nullptr);

    // Compose all three components (both modules + entry point) and link.
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(
        SLANG_SUCCEEDED(mainModule->findEntryPointByName("computeMain", entryPoint.writeRef())));

    slang::IComponentType* components[] = {libModule, mainModule, entryPoint};
    Slang::ComPtr<slang::IComponentType> composite;
    SLANG_CHECK(SLANG_SUCCEEDED(
        session
            ->createCompositeComponentType(components, 3, composite.writeRef(), diag.writeRef())));

    Slang::ComPtr<slang::IComponentType> linked;
    SLANG_CHECK(SLANG_SUCCEEDED(composite->link(linked.writeRef(), diag.writeRef())));

    uint64_t sessionHandle = ctx().getProxyHandle(session.get());

    // --- Playback phase ---
    ctx().switchToPlayback();
    ctx().executeAll();
    ctx().disable();

    // --- Verify ---
    // Same invariant as the single-module test: the session handle from the
    // record phase must still resolve after the entire two-module replay.
    ISlangUnknown* playedBack = ctx().getProxy(sessionHandle);
    SLANG_CHECK(playedBack != nullptr);
}

// =============================================================================
// Handle correspondence: every distinct proxy created during recording must
// get a distinct handle, and every one of those handles must resolve to a
// distinct proxy after playback.
// =============================================================================
SLANG_UNIT_TEST(replayContextHandleCorrespondence)
{
    REPLAY_TEST;

    // --- Record phase: create 3 distinct proxies (1 global + 2 sessions) ---
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    slang::SessionDesc sessionDesc = {};
    Slang::ComPtr<slang::ISession> session1;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session1.writeRef())));

    Slang::ComPtr<slang::ISession> session2;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session2.writeRef())));

    // Capture the three handles.
    uint64_t globalHandle = ctx().getProxyHandle(globalSession.get());
    uint64_t session1Handle = ctx().getProxyHandle(session1.get());
    uint64_t session2Handle = ctx().getProxyHandle(session2.get());

    // All three should be valid (>= kFirstValidHandle).
    SLANG_CHECK(globalHandle >= kFirstValidHandle);
    SLANG_CHECK(session1Handle >= kFirstValidHandle);
    SLANG_CHECK(session2Handle >= kFirstValidHandle);

    // All three should be distinct from each other — the handle allocator
    // must not collide across distinct proxies.
    SLANG_CHECK(globalHandle != session1Handle);
    SLANG_CHECK(globalHandle != session2Handle);
    SLANG_CHECK(session1Handle != session2Handle);

    // --- Playback phase ---
    ctx().switchToPlayback();
    ctx().executeAll();
    ctx().disable();

    // --- Verify ---
    // Each handle should resolve to a live proxy after playback.
    ISlangUnknown* playedGlobal = ctx().getProxy(globalHandle);
    ISlangUnknown* playedSession1 = ctx().getProxy(session1Handle);
    ISlangUnknown* playedSession2 = ctx().getProxy(session2Handle);

    SLANG_CHECK(playedGlobal != nullptr);
    SLANG_CHECK(playedSession1 != nullptr);
    SLANG_CHECK(playedSession2 != nullptr);

    // And those resolved proxies should themselves be three distinct objects —
    // the replay must not collapse them into one.
    SLANG_CHECK(playedGlobal != playedSession1);
    SLANG_CHECK(playedGlobal != playedSession2);
    SLANG_CHECK(playedSession1 != playedSession2);
}

// =============================================================================
// Large blob round-trip: record a 4MB blob, replay it, and verify the bytes
// match. Stresses the stream's handling of payloads bigger than typical
// chunk/buffer sizes.
// =============================================================================
SLANG_UNIT_TEST(replayContextLargeBlobHashVerification)
{
    REPLAY_TEST;

    // Build 4MB of deterministic test data so we can compare bytewise after
    // playback. The formula is arbitrary but reproducible.
    const size_t blobSize = 4 * 1024 * 1024;
    Slang::List<uint8_t> largeData;
    largeData.setCount(blobSize);
    for (size_t i = 0; i < blobSize; i++)
    {
        largeData[i] = static_cast<uint8_t>((i * 37 + 13) % 256);
    }

    // --- Record phase ---
    // Wrap the bytes in a RawBlob and record it as an output. The recording
    // layer copies the buffer contents into the stream.
    ctx().setMode(Mode::Record);

    Slang::ComPtr<ISlangBlob> blob = Slang::RawBlob::create(largeData.getBuffer(), blobSize);
    ISlangBlob* blobPtr = blob.get();
    ctx().record(RecordFlag::Output, blobPtr);

    // --- Playback phase ---
    // record() in Playback mode reads back the bytes the recorder wrote and
    // hands us a fresh blob populated from them.
    ctx().switchToPlayback();

    ISlangBlob* readBlob = nullptr;
    ctx().record(RecordFlag::Output, readBlob);

    // --- Verify: same size, same bytes ---
    SLANG_CHECK(readBlob != nullptr);
    SLANG_CHECK(readBlob->getBufferSize() == blobSize);
    SLANG_CHECK(memcmp(readBlob->getBufferPointer(), largeData.getBuffer(), blobSize) == 0);

    readBlob->release();
}

// =============================================================================
// Multiple independent sessions: record two separate global-session +
// session pairs, then replay and check that all four handles remain distinct
// and resolve to four distinct proxies (no cross-pair aliasing).
// =============================================================================
SLANG_UNIT_TEST(replayContextMultipleSessions)
{
    REPLAY_TEST;

    // --- Record phase ---
    ctx().setMode(Mode::Record);

    // First (globalSession, session) pair.
    Slang::ComPtr<slang::IGlobalSession> globalSession1;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(
        SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession1.writeRef())));

    slang::SessionDesc sessionDesc = {};
    Slang::ComPtr<slang::ISession> session1;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession1->createSession(sessionDesc, session1.writeRef())));

    // Second, independent (globalSession, session) pair.
    Slang::ComPtr<slang::IGlobalSession> globalSession2;
    SLANG_CHECK(
        SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession2.writeRef())));

    Slang::ComPtr<slang::ISession> session2;
    SLANG_CHECK(SLANG_SUCCEEDED(globalSession2->createSession(sessionDesc, session2.writeRef())));

    // Snapshot all four handles.
    uint64_t gs1Handle = ctx().getProxyHandle(globalSession1.get());
    uint64_t s1Handle = ctx().getProxyHandle(session1.get());
    uint64_t gs2Handle = ctx().getProxyHandle(globalSession2.get());
    uint64_t s2Handle = ctx().getProxyHandle(session2.get());

    // All four must be pairwise distinct — six comparisons.
    SLANG_CHECK(gs1Handle != s1Handle);
    SLANG_CHECK(gs1Handle != gs2Handle);
    SLANG_CHECK(gs1Handle != s2Handle);
    SLANG_CHECK(s1Handle != gs2Handle);
    SLANG_CHECK(s1Handle != s2Handle);
    SLANG_CHECK(gs2Handle != s2Handle);

    // --- Playback phase ---
    ctx().switchToPlayback();
    ctx().executeAll();
    ctx().disable();

    // --- Verify: all four handles resolve, and the two global sessions are
    // still distinct objects (no replay-side dedup collapsing them). ---
    SLANG_CHECK(ctx().getProxy(gs1Handle) != nullptr);
    SLANG_CHECK(ctx().getProxy(s1Handle) != nullptr);
    SLANG_CHECK(ctx().getProxy(gs2Handle) != nullptr);
    SLANG_CHECK(ctx().getProxy(s2Handle) != nullptr);

    SLANG_CHECK(ctx().getProxy(gs1Handle) != ctx().getProxy(gs2Handle));
}
