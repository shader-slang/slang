// unit-test-replay-modes.cpp
// Unit tests for ReplayContext mode and state management

#include "../../source/slang-record-replay/proxy/proxy-mutable-file-system.h"
#include "unit-test-replay-common.h"

// =============================================================================
// Stream State
// =============================================================================

SLANG_UNIT_TEST(replayContextStreamState)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Test isReading/isWriting
    ctx().reset();
    ctx().setMode(Mode::Record);
    SLANG_CHECK(ctx().isWriting());
    SLANG_CHECK(!ctx().isReading());

    int32_t value = 42;
    ctx().record(RecordFlag::None, value);

    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isReading());
    SLANG_CHECK(!ctx().isWriting());
}

// =============================================================================
// Proxy Wrapping Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextSessionWrappedWhenActive)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Save original state using the public C API
    bool wasActive = slang_isRecordLayerEnabled();

    // Enable replay context via the public C API
    slang_enableRecordLayer(true);
    SLANG_CHECK(slang_isRecordLayerEnabled() == true);

    // Create a global session via the public API
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SLANG_CHECK(globalSession != nullptr);

    // The session should be wrapped - verify by checking it's a GlobalSessionProxy
    // and has the correct ref count.
    SLANG_CHECK(dynamic_cast<GlobalSessionProxy*>(globalSession.get()) != nullptr);
    SLANG_CHECK(
        dynamic_cast<GlobalSessionProxy*>(globalSession.get())->debugGetReferenceCount() == 1);

    // Restore original state
    slang_enableRecordLayer(wasActive);
}

SLANG_UNIT_TEST(replayContextSessionNotWrappedWhenInactive)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Save original state using the public C API
    bool wasActive = slang_isRecordLayerEnabled();

    // Disable replay context via the public C API
    slang_enableRecordLayer(false);
    SLANG_CHECK(slang_isRecordLayerEnabled() == false);

    // Create a global session via the public API
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SLANG_CHECK(globalSession != nullptr);

    // The session should NOT be wrapped
    SLANG_CHECK(dynamic_cast<GlobalSessionProxy*>(globalSession.get()) == nullptr);

    // Restore original state
    slang_enableRecordLayer(wasActive);
}

// =============================================================================
// Mode Tests
// =============================================================================

SLANG_UNIT_TEST(replayContextIdleMode)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // In Idle mode, record operations should be no-ops
    ctx().reset();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(ctx().isIdle());
    SLANG_CHECK(!ctx().isActive());

    // Recording should not write anything
    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    SLANG_CHECK(ctx().getStream().getSize() == 0);

    // Multiple records should still produce no data
    float f = 3.14f;
    const char* str = "hello";
    ctx().record(RecordFlag::None, f);
    ctx().record(RecordFlag::None, str);
    SLANG_CHECK(ctx().getStream().getSize() == 0);
}

SLANG_UNIT_TEST(replayContextRecordMode)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // In Record mode, data should be written to stream
    ctx().reset();
    ctx().setMode(Mode::Record);
    SLANG_CHECK(ctx().getMode() == Mode::Record);
    SLANG_CHECK(ctx().isRecording());
    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().isWriting());

    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // Verify data was written correctly by reading it back
    ctx().switchToPlayback();
    int32_t readValue = 0;
    ctx().record(RecordFlag::None, readValue);
    SLANG_CHECK(readValue == 42);
}

SLANG_UNIT_TEST(replayContextPlaybackMode)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // First record some data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t writeInt = 123;
    float writeFloat = 2.5f;
    const char* writeStr = "test";

    ctx().record(RecordFlag::None, writeInt);
    ctx().record(RecordFlag::None, writeFloat);
    ctx().record(RecordFlag::None, writeStr);

    // Switch to playback and read data
    ctx().switchToPlayback();
    SLANG_CHECK(ctx().getMode() == Mode::Playback);
    SLANG_CHECK(ctx().isPlayback());
    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().isReading());

    int32_t readInt = 0;
    float readFloat = 0.0f;
    const char* readStr = nullptr;

    ctx().record(RecordFlag::None, readInt);
    ctx().record(RecordFlag::None, readFloat);
    ctx().record(RecordFlag::None, readStr);

    SLANG_CHECK(readInt == 123);
    SLANG_CHECK(readFloat == 2.5f);
    SLANG_CHECK(strcmp(readStr, "test") == 0);
}

SLANG_UNIT_TEST(replayContextSyncModeMatching)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // First, record reference data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t val1 = 100;
    int32_t val2 = 200;
    ctx().record(RecordFlag::None, val1);
    ctx().record(RecordFlag::None, val2);

    // Switch to sync mode - reset position and set mode
    ctx().switchToSync();
    SLANG_CHECK(ctx().getMode() == Mode::Sync);
    SLANG_CHECK(ctx().isSyncing());
    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().isWriting());

    // Record the same values - should succeed
    int32_t syncVal1 = 100;
    int32_t syncVal2 = 200;
    bool noException = true;
    try
    {
        ctx().record(RecordFlag::None, syncVal1);
        ctx().record(RecordFlag::None, syncVal2);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
}

SLANG_UNIT_TEST(replayContextSyncModeMismatch)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // First, record reference data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t val = 100;
    ctx().record(RecordFlag::None, val);

    // Switch to sync mode
    ctx().switchToSync();

    // Record a different value - should throw
    int32_t differentVal = 999;
    bool caughtException = false;
    try
    {
        ctx().record(RecordFlag::None, differentVal);
    }
    catch (const DataMismatchException& e)
    {
        caughtException = true;
        SLANG_CHECK(e.getSize() == sizeof(int32_t));
    }
    SLANG_CHECK(caughtException);
}

SLANG_UNIT_TEST(replayContextPlaybackOutputVerification)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Record data with Output flag
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t inputVal = 42;
    int32_t outputVal = 100;
    ctx().record(RecordFlag::Input, inputVal);
    ctx().record(RecordFlag::Output, outputVal);

    // Switch to playback
    ctx().switchToPlayback();

    // For inputs, we read the value from the stream (user provides 0, gets 42)
    int32_t readInput = 0;
    ctx().record(RecordFlag::Input, readInput);
    SLANG_CHECK(readInput == 42);

    // For outputs, user provides the expected value. Playback verifies it matches
    // the recorded value. If they match, no exception is thrown.
    int32_t expectedOutput = 100; // User says "I expect output to be 100"
    bool noException = true;
    try
    {
        ctx().record(RecordFlag::Output, expectedOutput);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
    SLANG_CHECK(expectedOutput == 100); // Value unchanged since it matched
}

SLANG_UNIT_TEST(replayContextPlaybackOutputMismatch)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Record an output value
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t outputVal = 100;
    ctx().record(RecordFlag::Output, outputVal);

    // Switch to playback
    ctx().switchToPlayback();

    // User provides wrong expected value - should throw
    int32_t wrongExpected = 999; // User says "I expect 999" but recorded was 100
    bool caughtException = false;
    try
    {
        ctx().record(RecordFlag::Output, wrongExpected);
    }
    catch (const DataMismatchException& e)
    {
        caughtException = true;
        SLANG_CHECK(e.getSize() == sizeof(int32_t));
    }
    SLANG_CHECK(caughtException);
}

SLANG_UNIT_TEST(replayContextModeTransitions)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().reset();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);

    // Test setMode transitions
    ctx().setMode(Mode::Record);
    SLANG_CHECK(ctx().getMode() == Mode::Record);
    SLANG_CHECK(ctx().isRecording());

    ctx().setMode(Mode::Idle);
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(ctx().isIdle());

    // Test enable() convenience method
    ctx().enable();
    SLANG_CHECK(ctx().getMode() == Mode::Record);

    // Test disable() convenience method
    ctx().disable();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);

    // enable() should only work from Idle
    ctx().setMode(Mode::Playback);
    ctx().enable(); // Should not change from Playback
    SLANG_CHECK(ctx().getMode() == Mode::Playback);
}

SLANG_UNIT_TEST(replayContextReset)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // Reset should clear everything
    ctx().reset();
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(ctx().getStream().getSize() == 0);
}

SLANG_UNIT_TEST(replayContextSyncModeWritesToStream)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Record reference data
    ctx().reset();
    ctx().setMode(Mode::Record);

    int32_t val = 42;
    ctx().record(RecordFlag::None, val);

    // Switch to sync mode
    ctx().switchToSync();

    // Sync mode should write to its own stream too
    int32_t syncVal = 42;
    ctx().record(RecordFlag::None, syncVal);

    // Verify sync context wrote to its stream
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // The written data should be readable
    ctx().switchToPlayback();
    int32_t readVal = 0;
    ctx().record(RecordFlag::None, readVal);
    SLANG_CHECK(readVal == 42);
}

// =============================================================================
// Test isolation: REPLAY_TEST must recover from a dirty singleton left behind
// by a prior test that aborted before its ScopedReplayContext dtor ran.
// =============================================================================

// Post-reset() invariants for the singleton. Both isolation tests below assert
// these, so keep the field set in sync with ReplayContext::reset() in
// source/slang-record-replay/replay-context.cpp.
static void checkReplayContextIsPristine()
{
    SLANG_CHECK(ctx().getMode() == Mode::Idle);
    SLANG_CHECK(!ctx().isActive());
    SLANG_CHECK(ctx().getStream().getSize() == 0);
    SLANG_CHECK(!ctx().hasCallIndex());
    SLANG_CHECK(ctx().getNextHandle() == kFirstValidHandle);
    SLANG_CHECK(ctx().getCurrentThisHandle() == kNullHandle);
}

// Build a dirty singleton state covering everything reset() clears that has an
// observable post-condition: a non-Idle mode, non-empty main and reference
// streams (via switchToSync), and entries in the handle dictionaries with
// m_nextHandle advanced past kFirstValidHandle (via registerProxy).
//
// The proxy registration must happen after switchToSync, since switchToSync
// itself wipes the dictionaries and resets m_nextHandle. The caller must keep
// the returned proxy alive across REPLAY_TEST so the dictionary entries are
// still present when ScopedReplayContext::reset() runs.
static Slang::ComPtr<MutableFileSystemProxy> dirtyReplayContext(int32_t value)
{
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, value);
    ctx().switchToSync();
    ctx().record(RecordFlag::None, value);

    auto* osFileSystem = Slang::OSFileSystem::getMutableSingleton();
    Slang::ComPtr<MutableFileSystemProxy> fsProxy(new MutableFileSystemProxy(osFileSystem));
    ctx().registerProxy(fsProxy.get(), osFileSystem);

    SLANG_CHECK(ctx().isActive());
    SLANG_CHECK(ctx().getStream().getSize() > 0);
    SLANG_CHECK(ctx().getNextHandle() > kFirstValidHandle);
    return fsProxy;
}

SLANG_UNIT_TEST(replayContextRecoversFromDirtyState)
{
    // Force a known-clean entry state. This test runs amid other replay tests
    // in the same process, and any prior test that left the stream in reading
    // mode (e.g. Mode::Playback) would make our setMode(Record) + record()
    // setup throw before REPLAY_TEST executes.
    ctx().reset();

    // Hold the proxy alive across REPLAY_TEST's ctor so the handle dictionaries
    // still contain entries when ScopedReplayContext::reset() runs.
    auto fsProxy = dirtyReplayContext(7);

    // ScopedReplayContext's ctor must force the singleton back to its
    // post-reset() state.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    checkReplayContextIsPristine();

    // And a normal record/playback cycle still works after recovery.
    ctx().setMode(Mode::Record);
    int32_t value = 123;
    ctx().record(RecordFlag::None, value);

    ctx().switchToPlayback();
    int32_t readBack = 0;
    ctx().record(RecordFlag::None, readBack);
    SLANG_CHECK(readBack == 123);
}

// Mode::Playback is the most failure-prone prior state: a prior test that
// aborted mid-read leaves the stream in reading mode at a non-zero position,
// and a subsequent test that calls setMode(Record) + record() before
// REPLAY_TEST throws on the still-reading stream.
SLANG_UNIT_TEST(replayContextRecoversFromDirtyPlaybackState)
{
    ctx().reset();

    // Record two values, switch to playback, consume the first one. The stream
    // is now in reading mode at a non-zero position, mimicking the state an
    // aborted SLANG_CHECK during a read would leave behind.
    ctx().setMode(Mode::Record);
    int32_t value = 42;
    ctx().record(RecordFlag::None, value);
    ctx().record(RecordFlag::None, value);
    ctx().switchToPlayback();
    int32_t consumed = 0;
    ctx().record(RecordFlag::None, consumed);
    SLANG_CHECK(consumed == 42);
    SLANG_CHECK(ctx().isPlayback());
    SLANG_CHECK(ctx().getStream().getPosition() > 0);

    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    checkReplayContextIsPristine();
}

// Dtor side of the REPLAY_TEST contract: when a test leaves the singleton
// dirty (mid-recording, non-empty streams, registered proxies) and exits the
// ScopedReplayContext scope normally, the dtor must reset it so the next test
// sees a clean singleton. Covered implicitly by every other replay test today,
// but pinning it down explicitly so a regression in
// ScopedReplayContext::~ScopedReplayContext fails here directly instead of as
// a downstream cascade.
SLANG_UNIT_TEST(replayContextDtorLeavesSingletonClean)
{
    ctx().reset();

    // Outer-scope ComPtr so the proxy outlives the inner ScopedReplayContext
    // dtor. The dictionaries must still hold the proxy entry when reset() runs
    // at inner-scope exit, otherwise the dictionary clears in reset() aren't
    // exercised.
    Slang::ComPtr<MutableFileSystemProxy> fsProxy;
    {
        REPLAY_TEST;
        SLANG_UNUSED(unitTestContext);

        fsProxy = dirtyReplayContext(99);
    }

    checkReplayContextIsPristine();
}
