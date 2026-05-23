// unit-test-replay-stream-corruption.cpp
//
// Stream-corruption rejection tests. loadReplay() reads stream.bin off disk
// without content validation, so each test writes an invalid stream.bin
// (random bytes / truncated recording) and asserts that executeAll() throws
// when it walks the bad bytes.

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <cstring>


// 8 bytes of garbage as stream.bin must throw from executeAll().
SLANG_UNIT_TEST(replayContextLoadReplayCorruptStream)
{
    REPLAY_TEST;

    // RAII: the scratch directory is removed on scope exit so a throw from
    // inside loadReplay() / executeAll() can't leak it into later tests.
    ScopedReplayDir scratch(".slang-replays-corrupt-test");

    // Eight arbitrary bytes that have no chance of parsing as a valid
    // record (no leading TypeId tag we recognize).
    Slang::String streamPath = Slang::Path::combine(scratch.path(), "stream.bin");
    const uint8_t garbage[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    Slang::File::writeAllBytes(streamPath, garbage, sizeof(garbage));

    SLANG_CHECK(SLANG_SUCCEEDED(ctx().loadReplay(scratch.path())));

    // executeAll() walks the records and trips on the garbage tag. Any
    // exception is acceptable; the test only verifies that no crash occurs
    // and that an exception is in fact raised.
    bool caughtException = false;
    try
    {
        ctx().executeAll();
    }
    catch (const Slang::Exception&)
    {
        caughtException = true;
    }
    SLANG_CHECK(caughtException);

    ctx().reset();
}

// A truncated real recording must throw cleanly mid-record from
// executeAll().
SLANG_UNIT_TEST(replayContextLoadReplayTruncatedStream)
{
    REPLAY_TEST;

    // Build a real recording in memory: createGlobalSession2 + findProfile.
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    SlangProfileID profile = globalSession->findProfile("sm_5_0");
    SLANG_CHECK(profile != SLANG_PROFILE_UNKNOWN);

    // RAII: the truncated-stream directory is removed on scope exit so a
    // throw mid-test can't leak it into later tests.
    ScopedReplayDir scratch(".slang-replays-truncated-test");

    // Snapshot the recorded bytes. The size lower-bound check ensures the
    // half-truncation below actually leaves at least a few bytes of valid
    // header in place (so the test exercises the mid-record failure
    // surface, not the empty-stream surface).
    const void* data = ctx().getStream().getData();
    size_t fullSize = ctx().getStream().getSize();
    SLANG_CHECK(fullSize > 10);

    // Write only the first half of the stream as stream.bin.
    Slang::String streamPath = Slang::Path::combine(scratch.path(), "stream.bin");
    Slang::File::writeAllBytes(streamPath, data, fullSize / 2);

    // Reset before loading so we don't try to play back the in-memory
    // recording instead of the truncated one on disk.
    ctx().reset();

    SLANG_CHECK(SLANG_SUCCEEDED(ctx().loadReplay(scratch.path())));

    // executeAll() walks records and trips when it tries to read past EOS
    // partway through a record. Any exception is acceptable.
    bool caughtException = false;
    try
    {
        ctx().executeAll();
    }
    catch (const Slang::Exception&)
    {
        caughtException = true;
    }
    SLANG_CHECK(caughtException);

    ctx().reset();
}
