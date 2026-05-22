#include "../../source/slang-record-replay/replay-stream-decoder.h"
#include "unit-test/slang-unit-test.h"

#include <cstdint>

using namespace Slang;
using namespace SlangRecord;

static bool containsString(const String& text, const char* value)
{
    return text.indexOf(UnownedStringSlice(value)) != -1;
}

static void writeReplayTypeId(ReplayStream& stream, TypeId type)
{
    uint8_t value = static_cast<uint8_t>(type);
    stream.write(&value, sizeof(value));
}

static void writeUInt64Payload(ReplayStream& stream, uint64_t value)
{
    stream.write(&value, sizeof(value));
}

static void writeArrayHeader(ReplayStream& stream, uint64_t count)
{
    writeReplayTypeId(stream, TypeId::Array);
    writeReplayTypeId(stream, TypeId::UInt64);
    writeUInt64Payload(stream, count);
}

SLANG_UNIT_TEST(replayStreamDecoderRejectsHugeArrayCount)
{
    ReplayStream stream;
    writeArrayHeader(stream, ~uint64_t(0));

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Array element count exceeds decoder limit"));
}

SLANG_UNIT_TEST(replayStreamDecoderBoundsSkippedArrayDepth)
{
    ReplayStream stream;
    writeArrayHeader(stream, 101);
    for (int i = 0; i < 100; ++i)
        writeReplayTypeId(stream, TypeId::Null);

    for (int i = 0; i < 70; ++i)
        writeArrayHeader(stream, 1);
    writeReplayTypeId(stream, TypeId::Null);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Maximum replay decode nesting depth exceeded"));
}

SLANG_UNIT_TEST(replayStreamDecoderBoundsDecodedArrayDepth)
{
    // Nest arrays deeper than kMaxReplayDecodeNestingDepth with count=1 at every
    // level so traversal stays on the decode path (i < kMaxDisplayedArrayElementCount)
    // and exercises checkReplayDecodeNestingDepth inside decodeValueFromStream,
    // complementing the skip-path coverage above.
    ReplayStream stream;
    for (int i = 0; i < 70; ++i)
        writeArrayHeader(stream, 1);
    writeReplayTypeId(stream, TypeId::Null);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Maximum replay decode nesting depth exceeded"));
}

SLANG_UNIT_TEST(replayStreamDecoderRejectsArrayLargerThanStream)
{
    // Count fits under the 1M element cap but exceeds the bytes remaining in the
    // stream, so validateReplayDecodeArrayElementCount must reach the
    // count > remainingBytes guard. This is the realistic attack: the cap alone
    // would still let a small-but-too-large count slip through.
    ReplayStream stream;
    writeArrayHeader(stream, 500);
    writeReplayTypeId(stream, TypeId::Null);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "exceeds remaining stream bytes"));
}

SLANG_UNIT_TEST(replayStreamDecoderRejectsInvalidArrayCountType)
{
    // The pre-PR code blindly skipped the count's TypeId byte. The new
    // readReplayDecodeArrayElementCount rejects anything other than UInt64;
    // pin that behavior so a future refactor cannot silently weaken it.
    ReplayStream stream;
    writeReplayTypeId(stream, TypeId::Array);
    writeReplayTypeId(stream, TypeId::UInt32);
    writeUInt64Payload(stream, 0);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Array element count has invalid type"));
}
