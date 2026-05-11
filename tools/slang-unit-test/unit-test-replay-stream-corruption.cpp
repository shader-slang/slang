// unit-test-replay-stream-corruption.cpp
//
// Stream-corruption rejection tests. Both tests load a stream.bin that is
// not a valid recording (random bytes, then a real but half-truncated
// recording) and verify executeAll() either rejects the stream cleanly via
// loadReplay() failure or surfaces a typed exception during execution.
//
// These don't require Fix C-1 (the noexcept-safe addRef/release path)
// because the failure happens inside executeAll() itself, not inside a
// destructor crossing the SLANG_NO_THROW boundary. Issue #10479 error-path
// bullet (loadReplay corruption / truncation cases).

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <cstring>
#include <vector>


// 8 bytes of garbage as stream.bin must surface as a clean exception.
SLANG_UNIT_TEST(replayContextLoadReplayCorruptStream)
{
    // loadReplay() doesn't validate content; it just reads bytes off disk
    // and only fails for IO-level errors (missing file, unreadable, etc.).
    // Format validation happens later, when executeAll() walks the typed
    // records. The test is defensive for the unlikely case that a future
    // change adds content validation to loadReplay(): if loadReplay starts
    // rejecting garbage up front, that's also acceptable.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Per-test directory holding the corrupt stream.bin.
    const char* testDir = ".slang-replays-corrupt-test";
    Slang::Path::createDirectoryRecursive(testDir);

    // Eight arbitrary bytes that have no chance of parsing as a valid
    // record (no leading TypeId tag we recognize).
    Slang::String streamPath = Slang::Path::combine(testDir, "stream.bin");
    const uint8_t garbage[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    Slang::File::writeAllBytes(streamPath, garbage, sizeof(garbage));

    // Try to load; loadReplay() should accept the bytes (no content
    // validation) but the test continues to be defensive against a future
    // change that adds validation here.
    SlangResult loadResult = ctx().loadReplay(testDir);

    if (SLANG_SUCCEEDED(loadResult))
    {
        // executeAll() walks the records and trips on the garbage tag.
        // Any exception is acceptable; the test only verifies that no
        // crash occurs and that an exception is in fact raised.
        bool caughtException = false;
        try
        {
            ctx().executeAll();
        }
        catch (...)
        {
            caughtException = true;
        }
        SLANG_CHECK(caughtException);
    }

    ctx().reset();
    Slang::Path::removeNonEmpty(testDir);
}

// A truncated real recording must throw cleanly mid-record.
SLANG_UNIT_TEST(replayContextLoadReplayTruncatedStream)
{
    // Same loadReplay caveat as above: no content validation, so
    // loadReplay should succeed and the throw happens inside executeAll
    // when it reads past EOS mid-record. Distinct from the truncation
    // sweep in unit-test-replay-fuzz.cpp (which sweeps every prefix
    // length): this pins one representative case for fast feedback.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Build a real recording in memory: createGlobalSession2 + findProfile.
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    SlangProfileID profile = globalSession->findProfile("sm_5_0");
    SLANG_CHECK(profile != SLANG_PROFILE_UNKNOWN);

    // Per-test directory for the truncated stream.bin.
    const char* testDir = ".slang-replays-truncated-test";
    Slang::Path::createDirectoryRecursive(testDir);

    // Snapshot the recorded bytes. The size lower-bound check ensures the
    // half-truncation below actually leaves at least a few bytes of valid
    // header in place (so the test exercises the mid-record failure
    // surface, not the empty-stream surface).
    const void* data = ctx().getStream().getData();
    size_t fullSize = ctx().getStream().getSize();
    SLANG_CHECK(fullSize > 10);

    // Write only the first half of the stream as stream.bin.
    Slang::String streamPath = Slang::Path::combine(testDir, "stream.bin");
    Slang::File::writeAllBytes(streamPath, data, fullSize / 2);

    // Reset before loading so we don't try to play back the in-memory
    // recording instead of the truncated one on disk.
    ctx().reset();

    SlangResult loadResult = ctx().loadReplay(testDir);
    if (SLANG_SUCCEEDED(loadResult))
    {
        // executeAll() walks records and trips when it tries to read past
        // EOS partway through a record. Any exception is acceptable.
        bool caughtException = false;
        try
        {
            ctx().executeAll();
        }
        catch (...)
        {
            caughtException = true;
        }
        SLANG_CHECK(caughtException);
    }

    ctx().reset();
    Slang::Path::removeNonEmpty(testDir);
}
