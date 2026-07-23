// unit-test-replay-error-paths.cpp
//
// Error-path and asymmetric recording/playback behavior for the replay system.

#include "../../source/slang-record-replay/proxy/proxy-global-session.h"
#include "../../source/slang-record-replay/proxy/proxy-session.h"
#include "unit-test-replay-common.h"

// =============================================================================
// Failed slang_createGlobalSession2: recording wraps unconditionally; playback
// guards with SLANG_SUCCEEDED (see replay-handlers.cpp handle_slang_createGlobalSession2).
// =============================================================================
SLANG_UNIT_TEST(replayContextFailedCreateGlobalSessionRecordingAndPlayback)
{
    REPLAY_TEST;

    // --- Record phase: force the call to fail ---
    // apiVersion must be 0; passing 1 makes slang_createGlobalSession2 return
    // a failure result and write nullptr to outGlobalSession. The recording
    // path runs anyway, capturing the call signature, the input desc, the
    // null output pointer, and the failure result.
    ctx().setMode(Mode::Record);

    SlangGlobalSessionDesc badDesc = {};
    badDesc.structureSize = sizeof(SlangGlobalSessionDesc);
    badDesc.apiVersion = 1;

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangResult recordResult = slang_createGlobalSession2(&badDesc, globalSession.writeRef());

    SLANG_CHECK(SLANG_FAILED(recordResult));
    SLANG_CHECK(globalSession == nullptr);

    // Something was actually written to the stream — empty stream would mean
    // the recording layer silently dropped the failed call.
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // --- Playback phase ---
    // The playback handler should consume the same bytes the recorder wrote
    // (signature + input + null output + failure result) without throwing,
    // and finish with the stream cursor at EOS.
    ctx().switchToPlayback();
    ctx().executeAll();
    SLANG_CHECK(ctx().getStream().atEnd());
    ctx().disable();
}

// =============================================================================
// Playback+Output: after executeAll, the session handle captured during
// recording must still resolve to a live SessionProxy. This is the core
// invariant Fix D (m_playbackKeepAlive) buys us — without it, replayed
// release() calls would tear down the proxy before the test can look it up.
// =============================================================================
SLANG_UNIT_TEST(replayContextPlaybackOutputSessionHandleCorrespondence)
{
    REPLAY_TEST;

    // --- Record phase ---
    ctx().setMode(Mode::Record);

    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;

    uint64_t sessionHandle = kNullHandle;
    {
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK(
            SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

        slang::SessionDesc sessionDesc = {};
        Slang::ComPtr<slang::ISession> session;
        SLANG_CHECK(SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
        SLANG_CHECK(session != nullptr);

        // Capture the recording-side handle for the session proxy. We'll
        // verify after playback that this same handle value still resolves
        // to a (newly-registered) proxy.
        sessionHandle = ctx().getProxyHandle(session.get());
        SLANG_CHECK(sessionHandle >= kFirstValidHandle);

        // Drop the record-side ComPtrs *before* switchToPlayback(). The mode
        // switch wipes the handle table; if a ComPtr destructs afterward, its
        // RECORD_CALL'd release() tries to look up the handle in the now-empty
        // table and throws. Explicit setNull() destructs while the table is
        // still populated.
        session.setNull();
        globalSession.setNull();
    }

    // --- Playback phase ---
    // executeAll() reads the recorded calls back, including the createSession
    // that re-registers a fresh session proxy and assigns it the same handle
    // value as during recording.
    ctx().switchToPlayback();
    ctx().executeAll();
    SLANG_CHECK(ctx().getStream().atEnd());

    // --- Verify ---
    // Look up by the recording-era handle. Should resolve to a SessionProxy
    // that was re-created during playback.
    ISlangUnknown* playedBackRaw = nullptr;
    try
    {
        playedBackRaw = ctx().getProxy(sessionHandle);
    }
    catch (const HandleNotFoundException&)
    {
        // Defensive: converts an opaque escaping exception into a readable
        // assertion-failure message if the invariant ever regresses.
        SLANG_CHECK_MSG(false, "playback did not re-register recorded session handle");
        ctx().disable();
        return;
    }
    SLANG_CHECK(playedBackRaw != nullptr);
    SLANG_CHECK(dynamic_cast<SessionProxy*>(playedBackRaw) != nullptr);

    ctx().disable();
}
