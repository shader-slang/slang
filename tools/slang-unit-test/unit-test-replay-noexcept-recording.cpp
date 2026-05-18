// unit-test-replay-noexcept-recording.cpp
//
// Tests that exercise the noexcept-safe addRef/release recording path. Every
// test here puts a Slang::ComPtr<> in a position where its destructor fires
// in non-Idle mode (Record / Sync / Playback) AND that destructor's release()
// triggers an exception inside the RECORD_CALL/record() path. Because the
// proxy's release() is declared SLANG_NO_THROW (and ~ComPtr is implicitly
// noexcept), any escape from that path calls std::terminate. The fix wraps
// the recording portion of PROXY_ADDREF_IMPL/PROXY_RELEASE_IMPL in
// try/catch and falls through to the underlying refcount op.

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <cstring>
#include <vector>

// =============================================================================
// Sync mode, matching arm: record a sequence, replay the same sequence in
// sync mode, expect zero divergence (no exception thrown).
// =============================================================================
SLANG_UNIT_TEST(replayContextSyncModeApiSequence)
{
    REPLAY_TEST;

    // --- Record phase ---
    // Capture a known-good API sequence into the reference stream: creating
    // a global session, then querying a profile by name.
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    SlangProfileID profile = globalSession->findProfile("sm_5_0");
    SLANG_CHECK(profile != SLANG_PROFILE_UNKNOWN);

    // --- Sync phase ---
    // switchToSync() moves the recorded bytes into a reference stream and
    // arms a comparator. Every subsequent record() call now reads N bytes
    // from the reference and verifies the live values match.
    ctx().switchToSync();
    SLANG_CHECK(ctx().isSyncing());

    // Re-issue the same calls with the same arguments. Each call's recorded
    // bytes should byte-for-byte match what's queued in the reference, so
    // no DataMismatchException is expected anywhere in the block — including
    // from the ComPtr destructor that fires at scope exit.
    bool noException = true;
    try
    {
        Slang::ComPtr<slang::IGlobalSession> syncGlobalSession;
        SLANG_CHECK(
            SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, syncGlobalSession.writeRef())));

        SlangProfileID syncProfile = syncGlobalSession->findProfile("sm_5_0");
        SLANG_CHECK(syncProfile == profile);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
}

// =============================================================================
// Sync mode, mismatch arm: record one findProfile argument, then in sync mode
// re-issue with a *different* argument. The mismatch must surface as an
// exception inside the try block — not as a std::terminate when ~ComPtr fires
// for the diverging proxy at scope exit.
// =============================================================================
SLANG_UNIT_TEST(replayContextSyncModeFindProfileMismatch)
{
    REPLAY_TEST;

    // --- Record phase ---
    // Reference recording uses "sm_5_0".
    ctx().setMode(Mode::Record);

    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    SlangProfileID profile = globalSession->findProfile("sm_5_0");
    SLANG_CHECK(profile != SLANG_PROFILE_UNKNOWN);

    // --- Sync phase ---
    ctx().switchToSync();
    SLANG_CHECK(ctx().isSyncing());

    bool caughtMismatch = false;
    try
    {
        // createGlobalSession2 with the same desc — matches the reference, no
        // mismatch here.
        Slang::ComPtr<slang::IGlobalSession> syncGlobal;
        SLANG_CHECK(
            SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, syncGlobal.writeRef())));

        // Deliberately diverge: reference recorded "sm_5_0", we ask for
        // "sm_6_0". The string argument bytes differ, so sync mode raises
        // an exception when that argument is record()'d.
        (void)syncGlobal->findProfile("sm_6_0");

        // syncGlobal's ~ComPtr fires here, in sync mode, mid-unwind. Without
        // the try/catch wrapper inside PROXY_RELEASE_IMPL, the sync-mode
        // divergence on that release escapes the noexcept boundary and calls
        // std::terminate. This destructor-during-unwind path is what Fix C
        // protects against.
    }
    catch (const DataMismatchException&)
    {
        // Payload-value mismatch — the most common form of divergence.
        caughtMismatch = true;
    }
    catch (const TypeMismatchException&)
    {
        // A sync-mode divergence can also surface as TypeMismatchException
        // when the diverging call's typed payload doesn't align with the
        // reference stream. Either is acceptable evidence that sync detected
        // the divergence.
        caughtMismatch = true;
    }
    SLANG_CHECK(caughtMismatch);
}
