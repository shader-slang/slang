// Diagnostic test for malformed character literals.
//
// Each line below contains a single bad character literal and asserts the
// exact diagnostic the lexer/parser produces for it. See
// docs/language-reference/expressions-literal.md for the literal format.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    // An empty character literal has no character value.
    uint a = '';
    //CHECK: illegal character literal

    // A character literal must hold exactly one character.
    uint b = 'ab';
    //CHECK: illegal character literal

    // '\q' is not a known escape, so '\' and 'q' are lexed as two separate
    // characters, which is too many for a character literal.
    uint c = '\q';
    //CHECK: illegal character literal

    // A '\u' escape requires exactly 4 hex digits.
    uint d = '\u12';
    //CHECK: \u must be followed by 4 hex digits

    // A '\U' escape requires exactly 8 hex digits.
    uint e = '\U1234';
    //CHECK: \U must be followed by 8 hex digits

    // A '\x' escape requires at least one hex digit.
    uint f = '\x';
    //CHECK: invalid string escape: '\x'

    // The braced forms require at least one hex digit between the braces.
    uint g = '\x{}';
    //CHECK: invalid string escape: '\x{}'

    uint h = '\u{}';
    //CHECK: invalid string escape: '\u{}'

    // A braced escape must be terminated with a closing brace.
    uint i = '\u{12';
    //CHECK: invalid string escape: '\u{12'

    // An overlong '\u' (more than 4 hex digits) is also wrong.
    uint j = '\u12345';
    //CHECK: \u must be followed by 4 hex digits

    // An overlong '\U' (more than 8 hex digits) is also wrong.
    uint k = '\U123456789';
    //CHECK: \U must be followed by 8 hex digits
}
