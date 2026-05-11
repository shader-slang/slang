// unit-test-replay-roundtrip-sync.cpp
//
// Lightweight round-trip / sync-mode tests that depend on no production-code
// fix. The sync-mode coverage here is deliberately scoped to scalar and string
// values; the API-call form of sync mode (where ComPtr destructors at scope
// exit can throw DataMismatchException through a SLANG_NO_THROW boundary)
// requires Fix C-1 and lives on replay-tests/fix-addref-noexcept.

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

// =============================================================================
// Error path handling
// =============================================================================

// loadReplay() on an obviously bogus path returns failure cleanly.
SLANG_UNIT_TEST(replayContextLoadReplayNonExistentPath)
{
    // Issue #10479 error-path bullet (the "non-existent path" case). The
    // corrupt-data and truncated-stream cases live in
    // unit-test-replay-stream-corruption.cpp.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Point at a path that definitely doesn't exist and try to load it.
    SlangResult result = ctx().loadReplay("/nonexistent/path/that/should/not/exist");

    // Result must be a SlangResult failure code (not an exception).
    SLANG_CHECK(SLANG_FAILED(result));

    // The context must still be in its non-playback state; a failed load
    // must not leave residual state that fakes a successful playback.
    SLANG_CHECK(!ctx().isPlayback());
}

// =============================================================================
// 3. Sync mode verification over scalar/string sequences
// =============================================================================

// Sync mode mismatch on string values raises DataMismatchException.
SLANG_UNIT_TEST(replayContextSyncModeStringMismatch)
{
    // Issue #10479 sync-mode bullet (mismatch-detection arm).
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

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
    // Issue #10479 sync-mode bullet (positive case): a faithful re-record
    // of the exact same values must NOT throw. Pairs with the mismatch
    // tests above and in unit-test-replay-modes.cpp to cover both arms of
    // sync-mode comparison.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

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
    // Issue #10479 edge-cases bullet. A regression that leaves the
    // caller's variable untouched (e.g., handle skipped because it was
    // "empty") would silently break any API that accepts nullable
    // interface arguments.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

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
