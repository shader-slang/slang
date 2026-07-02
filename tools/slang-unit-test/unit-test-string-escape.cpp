// unit-test-string-escape.cpp

#include "../../source/core/slang-string-escape-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static bool _checkConversion(StringEscapeHandler* handler, const UnownedStringSlice& check)
{
    StringBuilder buf;
    handler->appendEscaped(check, buf);

    StringBuilder decode;
    handler->appendUnescaped(buf.getUnownedSlice(), decode);

    return decode == check;
}

static bool _checkDecode(const UnownedStringSlice& encoded, const UnownedStringSlice& decoded)
{
    auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);

    StringBuilder buf;
    StringEscapeUtil::appendUnquoted(handler, encoded, buf);
    return buf == decoded;
}

#define SLANG_ENCODED_DECODED(x)      \
    const auto encoded = toSlice(#x); \
    const auto decoded = toSlice(x);

SLANG_UNIT_TEST(StringEscape)
{
    // Check greedy hex digits
    {
        // \x can have any number of hex digits
        const char text[] = "\x000001";
        SLANG_ASSERT(SLANG_COUNT_OF(text) == 2 && text[0] == 1);
    }

    // Check octal greedy
    {
        //\ + up to 3 octal digits
        const char text[] = "\0011";
        SLANG_ASSERT(SLANG_COUNT_OF(text) == 3 && text[0] == 1 && text[1] == '1');

        const char text2[] = "\78";
        SLANG_ASSERT(SLANG_COUNT_OF(text2) == 3 && text2[0] == 7 && text2[1] == '8');
    }

    {
        auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);

        SLANG_CHECK(_checkConversion(
            handler,
            toSlice("\0\1\2"
                    "2")));
    }

    {
        auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);

        // We can't just use '\uxxxx', because it has to be translatable into an output character in
        // MSVC (not into utf8) Can make work perhaps with something like #pragma
        // execution_character_set("utf-8") But for now we don't worry
        //
        // Visual Studio does not appear to support '\U' by default, presumably because wchar_t is
        // 16 bits

        {
            SLANG_ENCODED_DECODED("\a\b\0hey~\u0023\n\0");
            SLANG_CHECK(_checkDecode(encoded, decoded));
        }

        {
            SLANG_ENCODED_DECODED("\n\v\b\t\1\02\003\x5z\x00007f\0");
            SLANG_CHECK(_checkDecode(encoded, decoded));
        }
    }

    // JSON has only 16-bit \u escapes, so code points >= 0x10000 must round-trip
    // through a UTF-16 surrogate pair. Regression test for astral characters
    // (e.g. emoji and the maximum code point U+10FFFD) being truncated to 16 bits.
    {
        auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON);

        // BMP characters and astral characters (as raw UTF-8 bytes) must survive
        // an escape/unescape round trip unchanged. The literals below are spelled
        // out as explicit UTF-8 byte sequences (ä=C3A4, €=E282AC, ☺=E298BA,
        // 😊 U+1F60A=F09F988A, U+10FFFD=F48FBFBD) so the test does not depend on
        // the source file's encoding.
        SLANG_CHECK(_checkConversion(handler, toSlice("\xC3\xA4\xE2\x82\xAC\xE2\x98\xBA")));
        SLANG_CHECK(_checkConversion(handler, toSlice("\xF0\x9F\x98\x8A")));
        SLANG_CHECK(_checkConversion(
            handler,
            toSlice("\xF0\x9F\x98\x8A\xC3\xA4\xE2\x82\xAC\xF4\x8F\xBF\xBD")));

        // An astral code point must be escaped as a surrogate pair (two \u
        // escapes), not a single (truncated) \u escape. U+1F60A == D83D DE0A.
        {
            StringBuilder buf;
            handler->appendEscaped(toSlice("\xF0\x9F\x98\x8A"), buf);
            SLANG_CHECK(buf.getUnownedSlice() == toSlice("\\ud83d\\ude0a"));
        }
    }

    // Regression test for #11864: decoding a truncated \u escape, or an
    // unpaired high surrogate at the very end of the input, must not form a
    // pointer more than one past the end of the buffer while probing for the
    // remaining bytes. These inputs exercise the two buffer-end guards in
    // JSONStringEscapeHandler::appendUnescaped(); the accept/reject decisions
    // asserted below must hold, and under UBSan no out-of-bounds pointer is
    // formed.
    {
        auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::JSON);

        // A \u escape with fewer than four hex digits remaining is rejected
        // (this guards the four-hex-digit read).
        {
            StringBuilder decode;
            SLANG_CHECK(SLANG_FAILED(handler->appendUnescaped(toSlice("\\u"), decode)));
        }
        {
            StringBuilder decode;
            SLANG_CHECK(SLANG_FAILED(handler->appendUnescaped(toSlice("\\u12"), decode)));
        }

        // An unpaired high surrogate at the end of the input leaves no room for
        // a following low-surrogate \u escape, so the surrogate-pair combine is
        // skipped and the lone high surrogate is emitted on its own. The lone
        // surrogate U+D800 is UTF-8 encoded on its own as the three bytes ED A0 80
        // (this is WTF-8; the decoder does not reject or replace lone surrogates).
        {
            StringBuilder decode;
            SLANG_CHECK(SLANG_SUCCEEDED(handler->appendUnescaped(toSlice("\\uD800"), decode)));
            SLANG_CHECK(decode.getUnownedSlice() == toSlice("\xED\xA0\x80"));
        }

        // A high surrogate followed by a truncated second \u escape (fewer than
        // four hex digits) must be rejected. The surrogate-pair look-ahead is
        // skipped (no room for six more bytes), the lone high surrogate is
        // emitted, and then the truncated trailing \u escape fails the four-hex
        // digit guard, so the whole decode returns failure.
        {
            StringBuilder decode;
            SLANG_CHECK(SLANG_FAILED(handler->appendUnescaped(toSlice("\\uD83D\\uDE"), decode)));
        }

        // A complete surrogate pair still combines into the single astral code
        // point U+1F60A (UTF-8 F0 9F 98 8A), confirming the six-byte look-ahead
        // accept path survives the guard rewrite.
        {
            StringBuilder decode;
            SLANG_CHECK(
                SLANG_SUCCEEDED(handler->appendUnescaped(toSlice("\\uD83D\\uDE0A"), decode)));
            SLANG_CHECK(decode.getUnownedSlice() == toSlice("\xF0\x9F\x98\x8A"));
        }
    }
}
