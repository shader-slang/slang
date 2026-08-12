// Diagnostic test for malformed escape sequences in string literals
//
// See docs/language-reference/expressions-literal.md for the literal format.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    // A '\u' escape requires exactly 4 hex digits.
    String a = "ab\u12xy";
    //CHECK: \u must be followed by 4 hex digits

    // A '\U' escape requires exactly 8 hex digits.
    String b = "ab\U1234xy";
    //CHECK: \U must be followed by 8 hex digits

    // A '\x' escape requires at least one hex digit.
    String c = "ab\xxy";
    //CHECK: invalid string escape: '\x'

    // The braced forms require at least one hex digit between the braces.
    String d = "ab\x{}xy";
    //CHECK: invalid string escape: '\x{}'

    String e = "ab\u{}xy";
    //CHECK: invalid string escape: '\u{}'

    // A braced escape must be terminated with a closing brace.
    String f = "ab\u{12xy";
    //CHECK: invalid string escape: '\u{12'

    // '\q' is not a known escape sequence.
    String g = "\q";
    //CHECK: invalid string escape: '\q'

    // A '\x' value must fit in 32 bits; this one has nine hex digits.
    String h = "\x100000000";
    //CHECK: invalid string escape: '\x100000000'

    // In a string literal a '\x' byte escape must be in the range 0-255.
    String i = "\xFEDC";
    //CHECK: code unit 65244 (0xFEDC) is out of range

    // A Unicode code point above 0x1FFFFF cannot be encoded as UTF-8.
    String j = "\U00200000";
    //CHECK: Unicode code point 2097152 (0x200000) cannot be encoded as UTF-8
}
