// Diagnostic test for malformed escape sequences in string literals.
//
// String-literal escapes are validated when the literal is decoded (during
// parsing), not during lexing: a string-literal token may in fact be a
// quote-form #include path, which is consumed raw and never decoded, so the
// lexer must not validate escapes in it (see #11829). These mirror the cases in
// char-literal-errors.slang -- character literals are never include paths, so
// those are validated at lex time. Unknown-escape and out-of-range-value cases
// live in string-literal-errors-value.slang.
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
    // An overlong '\u' (more than 4 hex digits) is also wrong.
    String g = "\u12345";
    //CHECK: \u must be followed by 4 hex digits

    // An overlong '\U' (more than 8 hex digits) is also wrong.
    String h = "\U123456789";
    //CHECK: \U must be followed by 8 hex digits

}
