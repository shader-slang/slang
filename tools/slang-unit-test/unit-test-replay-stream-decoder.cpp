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
