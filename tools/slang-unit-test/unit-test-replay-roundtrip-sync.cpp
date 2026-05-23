// unit-test-replay-roundtrip-sync.cpp
//
// Lightweight round-trip / sync-mode tests. The sync-mode coverage here is
// scoped to scalar and string values driven directly through ctx().record();
// sync-mode coverage of the proxy API-call path is not exercised here.

#include "unit-test-replay-common.h"

// =============================================================================
// Error path handling
// =============================================================================

// loadReplay() on an obviously bogus path returns failure cleanly.
SLANG_UNIT_TEST(replayContextLoadReplayNonExistentPath)
{
    // The corrupt-data and truncated-stream variants of the failure-path
    // contract live in unit-test-replay-stream-corruption.cpp.
    REPLAY_TEST;

    // Snapshot the pre-call state so we can verify the failure path didn't
    // mutate anything it shouldn't have.
    size_t streamSizeBefore = ctx().getStream().getSize();
    Mode modeBefore = ctx().getMode();

    // Point at a path that definitely doesn't exist and try to load it.
    SlangResult result = ctx().loadReplay("/nonexistent/path/that/should/not/exist");

    // Result must be a SlangResult failure code (not an exception).
    SLANG_CHECK(SLANG_FAILED(result));

    // The context must still be in its pre-call state. A regression that
    // half-applied the load (left m_mode flipped, populated handle tables,
    // appended bytes to the stream, or merely flipped out of Idle) would
    // get past a bare !isPlayback() check; comparing mode and stream size
    // to the pre-call snapshot pins all those down. REPLAY_TEST guarantees
    // modeBefore == Mode::Idle.
    SLANG_CHECK(ctx().getMode() == modeBefore);
    SLANG_CHECK(ctx().isIdle());
    SLANG_CHECK(!ctx().isPlayback());
    SLANG_CHECK(ctx().getStream().getSize() == streamSizeBefore);
}

// =============================================================================
// 3. Sync mode verification over scalar/string sequences
// =============================================================================

// Sync mode mismatch on string values raises DataMismatchException.
SLANG_UNIT_TEST(replayContextSyncModeStringMismatch)
{
    REPLAY_TEST;

    // Record a reference string. The stream now contains a single String
    // entry with this value.
    ctx().setMode(Mode::Record);
    const char* str = "original_string";
    ctx().record(RecordFlag::None, str);

    // Switch to sync mode. Subsequent record() calls re-read the stream
    // and validate that the value being "recorded" matches what's already
    // there.
    ctx().switchToSync();

    // Try to record a different string. The sync layer should compare it
    // against "original_string", find a mismatch, and throw.
    const char* differentStr = "different_string";
    bool caughtException = false;
    try
    {
        ctx().record(RecordFlag::None, differentStr);
    }
    catch (const DataMismatchException&)
    {
        caughtException = true;
    }
    SLANG_CHECK(caughtException);
}

// Sync mode match path over a heterogeneous scalar+string sequence.
SLANG_UNIT_TEST(replayContextSyncModeMultipleValuesMatch)
{
    // Positive case for sync mode: a faithful re-record of the exact same
    // values must NOT throw. Pairs with the mismatch tests above and in
    // unit-test-replay-modes.cpp to cover both arms of sync-mode comparison.
    REPLAY_TEST;

    ctx().setMode(Mode::Record);

    // Reference values, one per supported scalar/string TypeId so the test
    // covers the multi-record sync path rather than just one comparison.
    int32_t val1 = 42;
    float val2 = 3.14f;
    const char* val3 = "hello";
    uint64_t val4 = 0xDEADBEEFCAFEBABEULL;

    ctx().record(RecordFlag::None, val1);
    ctx().record(RecordFlag::None, val2);
    ctx().record(RecordFlag::None, val3);
    ctx().record(RecordFlag::None, val4);

    // Switch to sync and re-issue the exact same record() calls in the
    // same order with bitwise-identical values.
    ctx().switchToSync();

    int32_t sync1 = 42;
    float sync2 = 3.14f;
    const char* sync3 = "hello";
    uint64_t sync4 = 0xDEADBEEFCAFEBABEULL;

    // None of the four calls may throw. Catching DataMismatchException
    // specifically (rather than `...`) keeps the failure attribution clean
    // if some unrelated exception ever shows up.
    bool noException = true;
    try
    {
        ctx().record(RecordFlag::None, sync1);
        ctx().record(RecordFlag::None, sync2);
        ctx().record(RecordFlag::None, sync3);
        ctx().record(RecordFlag::None, sync4);
    }
    catch (const DataMismatchException&)
    {
        noException = false;
    }
    SLANG_CHECK(noException);
}

// Null handles round-trip cleanly through record then playback.
SLANG_UNIT_TEST(replayContextNullHandleRoundTrip)
{
    // A regression that leaves the caller's variable untouched (e.g.,
    // handle skipped because it was "empty") would silently break any API
    // that accepts nullable interface arguments.
    REPLAY_TEST;

    ctx().setMode(Mode::Record);

    // Record kNullHandle.
    uint64_t writeHandle = kNullHandle;
    ctx().recordHandle(RecordFlag::Input, writeHandle);

    ctx().switchToPlayback();

    // Read into a deliberately non-null sentinel so a missed-write
    // regression would leave 0xFFFF... in place rather than coincidentally
    // matching kNullHandle.
    uint64_t readHandle = 0xFFFFFFFFFFFFFFFFULL;
    ctx().recordHandle(RecordFlag::Input, readHandle);

    // Playback must have overwritten the sentinel with kNullHandle.
    SLANG_CHECK(readHandle == kNullHandle);
}
