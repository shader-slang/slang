// Diagnostic test for character literals containing illegal UTF-8 byte
// sequences that fail to decode to a Unicode code point.
//
// A lone UTF-8 continuation byte (0x80-0xBF) is not a valid leading byte,
// so the lexer passes it through as a single-byte character token and
// getCharLiteralValue then rejects it while decoding the UTF-8. Like the
// out-of-range value case, this is diagnosed during parsing, so these
// literals must be the only errors in the file (no lexer-phase errors).
//
// This file intentionally contains raw non-UTF-8 bytes; do not let an
// editor "fix" the encoding.
//
// See docs/language-reference/expressions-literal.md for the literal format.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    // Smallest continuation byte.
    uint a = '€';
    //CHECK: invalid UTF-8 byte sequence: 0x80

    // A continuation byte in the middle of the range.
    uint b = '¥';
    //CHECK: invalid UTF-8 byte sequence: 0xA5

    // Largest continuation byte.
    uint c = '¿';
    //CHECK: invalid UTF-8 byte sequence: 0xBF
}
