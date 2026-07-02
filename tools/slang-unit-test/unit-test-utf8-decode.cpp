// unit-test-utf8-decode.cpp

#include "../../source/core/slang-char-encode.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// Decode a fixed byte buffer as a single UTF-8 code point, reporting whether
// the sequence was invalid via outInvalid.
//
// Bytes read beyond the end of the buffer are reported as 0xFF. 0xFF is never a
// valid continuation byte, so a sequence that announces more bytes than the
// buffer holds is detected as invalid. This mirrors how getCharLiteralValue
// feeds the decoder a sentinel past the end of the literal's content.
Char32 decodeUtf8(const unsigned char* bytes, size_t count, bool& outInvalid)
{
    size_t cursor = 0;
    return getUnicodePointFromUTF8(
        [&]() -> unsigned char { return cursor < count ? bytes[cursor++] : 0xFFu; },
        &outInvalid);
}
} // namespace

SLANG_UNIT_TEST(utf8Decode)
{
    bool invalid = false;

    // Valid sequences round-trip to the expected code point and are not flagged.
    {
        const unsigned char ascii[] = {0x41}; // 'A'
        SLANG_CHECK(decodeUtf8(ascii, 1, invalid) == 0x41u && !invalid);

        const unsigned char two[] = {0xC3, 0xA4}; // 'ä' U+00E4
        SLANG_CHECK(decodeUtf8(two, 2, invalid) == 0xE4u && !invalid);

        const unsigned char three[] = {0xE2, 0x82, 0xAC}; // '€' U+20AC
        SLANG_CHECK(decodeUtf8(three, 3, invalid) == 0x20ACu && !invalid);

        const unsigned char four[] = {0xF0, 0x9F, 0x98, 0x8A}; // '😊' U+1F60A
        SLANG_CHECK(decodeUtf8(four, 4, invalid) == 0x1F60Au && !invalid);
    }

    // A lone continuation byte cannot begin a sequence.
    {
        const unsigned char cont[] = {0x80};
        decodeUtf8(cont, 1, invalid);
        SLANG_CHECK(invalid);
    }

    // (1) Multibyte sequences that do not have enough continuation bytes.
    {
        // 0xE0 announces a 3-byte sequence, but the next byte 0x41 ('A') is not
        // a continuation byte.
        const unsigned char nonContinuation[] = {0xE0, 0x41};
        decodeUtf8(nonContinuation, 2, invalid);
        SLANG_CHECK(invalid);

        // 0xF0 announces a 4-byte sequence but only two bytes are present; the
        // missing continuation bytes are read as the 0xFF sentinel.
        const unsigned char truncated[] = {0xF0, 0x9F};
        decodeUtf8(truncated, 2, invalid);
        SLANG_CHECK(invalid);
    }

    // (2) Over-encoded (overlong) sequences: structurally valid leading and
    // continuation bytes, but encoding a value below the minimum for that
    // length, so a shorter encoding exists.
    {
        // U+0000 in two bytes (the minimum for 2 bytes is U+0080).
        const unsigned char overlong2[] = {0xC0, 0x80};
        decodeUtf8(overlong2, 2, invalid);
        SLANG_CHECK(invalid);

        // U+0000 in three bytes (the minimum for 3 bytes is U+0800).
        const unsigned char overlong3[] = {0xE0, 0x80, 0x80};
        decodeUtf8(overlong3, 3, invalid);
        SLANG_CHECK(invalid);

        // U+20AC ('€') in four bytes — it fits in three (E2 82 AC), and the
        // minimum for 4 bytes is U+10000.
        const unsigned char overlong4[] = {0xF0, 0x82, 0x82, 0xAC};
        decodeUtf8(overlong4, 4, invalid);
        SLANG_CHECK(invalid);
    }
}
