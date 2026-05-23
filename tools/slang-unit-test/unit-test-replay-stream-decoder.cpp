#include "slang-record-replay/replay-stream-decoder.h"
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

static void writeUInt32Value(ReplayStream& stream, uint32_t value)
{
    writeReplayTypeId(stream, TypeId::UInt32);
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
    // Pin the 1,000,000-element cap at the boundary: count = cap + 1 trips the
    // first guard in validateReplayDecodeArrayElementCount with the cap-specific
    // error string. A regression that bumps the cap or reorders the cap check
    // behind the remaining-bytes check would either silence this error string
    // entirely or change which guard fires first, breaking the test.
    ReplayStream stream;
    writeArrayHeader(stream, 1000001);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Array element count exceeds decoder limit"));
}

SLANG_UNIT_TEST(replayStreamDecoderBoundsSkippedArrayDepth)
{
    // Pin the 64-depth boundary on the skip path. The outer Array[101] places
    // skipValueInStream calls at depth=1 (since decodeValueFromStream starts at
    // depth=0). 64 nested Array headers inside the skip-loop payload mean the
    // recursive skipValueInStream call at depth=65 trips
    // `recursionDepth > kMaxReplayDecodeNestingDepth` before reading any byte.
    // Changing the operator from `>` to `>=` (off-by-one regression) would shift
    // the trip to depth=64 and let this test pass at depth=63 — so any rewrite
    // of either the constant or the operator breaks this test.
    ReplayStream stream;
    writeArrayHeader(stream, 101);
    for (int i = 0; i < 100; ++i)
        writeReplayTypeId(stream, TypeId::Null);

    for (int i = 0; i < 64; ++i)
        writeArrayHeader(stream, 1);
    writeReplayTypeId(stream, TypeId::Null);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Maximum replay decode nesting depth exceeded"));
}

SLANG_UNIT_TEST(replayStreamDecoderBoundsDecodedArrayDepth)
{
    // Pin the 64-depth boundary on the decode path. decodeValueFromStream
    // starts at depth=0, so 65 nested Array[1] headers produce a call at
    // depth=65, which trips `recursionDepth > kMaxReplayDecodeNestingDepth`
    // before reading. Companion to the at-max-depth happy-path test below
    // (64 levels must NOT trip the same guard).
    ReplayStream stream;
    for (int i = 0; i < 65; ++i)
        writeArrayHeader(stream, 1);
    writeReplayTypeId(stream, TypeId::Null);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Maximum replay decode nesting depth exceeded"));
}

SLANG_UNIT_TEST(replayStreamDecoderAcceptsArraysAtMaxDepth)
{
    // Companion to the boundary test above: 64 nested Array[1] headers reach
    // exactly depth=64 (`recursionDepth > 64` is false). The decode must
    // succeed and the depth-exceeded error message must NOT appear. If the
    // operator were tightened to `>=`, this test would start failing.
    ReplayStream stream;
    for (int i = 0; i < 64; ++i)
        writeArrayHeader(stream, 1);
    writeReplayTypeId(stream, TypeId::Null);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(!containsString(decoded, "Maximum replay decode nesting depth exceeded"));
    SLANG_CHECK(!containsString(decoded, "ERROR:"));
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

SLANG_UNIT_TEST(replayStreamDecoderRejectsHugeArrayCountOnSkipPath)
{
    // The decode-path cap test puts the oversized Array[1000001] at top level,
    // which is decode (not skip). Mirror the depth-on-skip-path pattern: outer
    // Array[101] forces element 101 through skipValueInStream, where the
    // nested oversized array trips `count > kMaxReplayDecodeArrayElementCount`.
    // A regression that splits readReplayDecodeArrayElementCount and weakens
    // the cap on only the skip path would otherwise sneak by.
    ReplayStream stream;
    writeArrayHeader(stream, 101);
    for (int i = 0; i < 100; ++i)
        writeReplayTypeId(stream, TypeId::Null);
    writeArrayHeader(stream, 1000001);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Array element count exceeds decoder limit"));
}

SLANG_UNIT_TEST(replayStreamDecoderDecodesValidArray)
{
    // Happy path: a valid Array of three UInt32 elements must decode cleanly,
    // with the element values visible in the output and no error string. This
    // guards against an over-eager early-throw added to a future bound check
    // that would silently regress legitimate inputs.
    ReplayStream stream;
    writeArrayHeader(stream, 3);
    writeUInt32Value(stream, 11);
    writeUInt32Value(stream, 22);
    writeUInt32Value(stream, 33);

    ReplayStream reader = stream.createReader();
    String decoded = ReplayStreamDecoder::decode(reader);

    SLANG_CHECK(containsString(decoded, "Array[3]:"));
    SLANG_CHECK(containsString(decoded, "UInt32: 11"));
    SLANG_CHECK(containsString(decoded, "UInt32: 22"));
    SLANG_CHECK(containsString(decoded, "UInt32: 33"));
    SLANG_CHECK(!containsString(decoded, "ERROR:"));
    SLANG_CHECK(!containsString(decoded, "exceeds"));
}
