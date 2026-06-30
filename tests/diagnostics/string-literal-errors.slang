// Diagnostic test for malformed escape sequences in string literals that are
// rejected during lexing.
//
// These mirror the lexer-phase cases in char-literal-errors.slang. Unlike a
// character literal, a string literal has no single-character constraint, so an
// unknown escape or an out-of-range escape value is instead diagnosed during
// parsing -- those cases live in string-literal-errors-value.slang.
//
// See docs/language-reference/expressions-literal.md for the literal format.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    // A '\u' escape requires exactly 4 hex digits.
    String a = "\u12";
    //CHECK: \u must be followed by 4 hex digits

    // A '\U' escape requires exactly 8 hex digits.
    String b = "\U1234";
    //CHECK: \U must be followed by 8 hex digits

    // A '\x' escape requires at least one hex digit.
    String c = "\x";
    //CHECK: invalid string escape: '\x'

    // The braced forms require at least one hex digit between the braces.
    String d = "\x{}";
    //CHECK: invalid string escape: '\x{}'

    String e = "\u{}";
    //CHECK: invalid string escape: '\u{}'

    // A braced escape must be terminated with a closing brace.
    String f = "\u{12";
    //CHECK: invalid string escape: '\u{12'
}
