// unit-test-slang-embed.cpp
//
// Tests that the slang-embed octal escape encoding correctly handles non-ASCII
// bytes (0x80-0xFF) without sign-extension corruption.

#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <string.h>

// Reproduce the encoding logic from tools/slang-embed/slang-embed.cpp.
// Non-printable bytes are emitted as three-digit octal escape sequences.
// The key fix is casting to `unsigned char` before passing to fprintf/snprintf
// to prevent sign-extension on platforms where `char` is signed.
static void encodeByteAsOctal(char c, char* buf, size_t bufSize)
{
    snprintf(buf, bufSize, "\\%03o", (unsigned char)c);
}

// Reproduce the original buggy logic (no cast) to confirm it produces
// sign-extended output on platforms where char is signed.
static void encodeByteAsOctalBuggy(char c, char* buf, size_t bufSize)
{
    snprintf(buf, bufSize, "\\%03o", c);
}

SLANG_UNIT_TEST(slangEmbedOctalEscape)
{
    char buf[32];

    // The UTF-8 encoding of the em dash U+2014 is three bytes: 0xE2, 0x80, 0x94.
    // On signed-char platforms these are negative values (-30, -128, -108).
    // Without the (unsigned char) cast they would sign-extend to large 32-bit
    // values and produce oversized octal literals such as \37777777742.

    // 0xE2 == 226 == octal 342
    encodeByteAsOctal((char)0xE2, buf, sizeof(buf));
    SLANG_CHECK(strcmp(buf, "\\342") == 0);

    // 0x80 == 128 == octal 200
    encodeByteAsOctal((char)0x80, buf, sizeof(buf));
    SLANG_CHECK(strcmp(buf, "\\200") == 0);

    // 0x94 == 148 == octal 224
    encodeByteAsOctal((char)0x94, buf, sizeof(buf));
    SLANG_CHECK(strcmp(buf, "\\224") == 0);

    // 0xFF == 255 == octal 377 (maximum byte value)
    encodeByteAsOctal((char)0xFF, buf, sizeof(buf));
    SLANG_CHECK(strcmp(buf, "\\377") == 0);

    // 0x81 == 129 == octal 201 (minimum non-ASCII value + 1)
    encodeByteAsOctal((char)0x81, buf, sizeof(buf));
    SLANG_CHECK(strcmp(buf, "\\201") == 0);

    // Verify ASCII bytes still encode correctly (0x1B == 27 == octal 033)
    encodeByteAsOctal('\x1B', buf, sizeof(buf));
    SLANG_CHECK(strcmp(buf, "\\033") == 0);

    // Negative test: reproduce the original bug to confirm detection.
    // On platforms where char is signed, passing a non-ASCII byte without the
    // (unsigned char) cast sign-extends the value to int, so %o formats it as
    // a large number (e.g. 0xFFFFFFE2 == 037777777742 for 0xE2).
    // The result must NOT be a compact 3-digit escape.
    if ((char)0xFF < 0)
    {
        // char is signed on this platform: verify the bug actually manifests.
        encodeByteAsOctalBuggy((char)0xE2, buf, sizeof(buf));
        SLANG_CHECK(strcmp(buf, "\\342") != 0); // must be oversized, not \342
        SLANG_CHECK(strlen(buf) > 4);           // oversized escape is longer than \NNN
    }
}
