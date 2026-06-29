// Diagnostic test for string-literal escapes whose value is invalid.
//
// These are diagnosed during parsing (by getStringLiteralTokenValue), not
// during lexing. As with char-literal-errors-value.slang, such a diagnostic is
// suppressed once the file already contains lexer-phase errors, so these
// escapes must be the only errors present.
//
// In a string literal an octal/hex escape encodes an individual byte (range
// 0-255), while a Unicode escape is expanded to its UTF-8 byte sequence; both
// have value constraints that lexing does not check.
//
// See docs/language-reference/expressions-literal.md for the literal format.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    // '\q' is not a known escape sequence.
    String a = "\q";
    //CHECK: invalid string escape: '\q'

    // A '\x' value must fit in 32 bits; this one has nine hex digits.
    String b = "\x100000000";
    //CHECK: invalid string escape: '\x100000000'

    // In a string literal a '\x' byte escape must be in the range 0-255.
    String c = "\xFEDC";
    //CHECK: code unit 65244 (0xFEDC) is out of range

    // A Unicode code point above 0x1FFFFF cannot be encoded as UTF-8.
    String d = "\U00200000";
    //CHECK: Unicode code point 2097152 (0x200000) cannot be encoded as UTF-8
}
