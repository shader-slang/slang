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
//CHECK:     ^^ illegal character literal

    // A character literal must hold exactly one character.
    uint b = 'ab';
//CHECK:     ^^^^ illegal character literal

    // '\q' is not a known escape
    uint c = '\q';
//CHECK:     ^^^^ invalid string escape: '\q'

    // A '\u' escape requires exactly 4 hex digits.
    uint d = '\u12';
//CHECK:     ^^^^^^ invalid string escape: \u must be followed by 4 hex digits

    // A '\U' escape requires exactly 8 hex digits.
    uint e = '\U1234';
//CHECK:     ^^^^^^^^ invalid string escape: \U must be followed by 8 hex digits

    // A '\x' escape requires at least one hex digit.
    uint f = '\x';
//CHECK:     ^^^^ invalid string escape: '\x'

    // The braced forms require at least one hex digit between the braces.
    uint g0 = '\x{';
//CHECK:      ^^^^^ invalid string escape: '\x{'

    // The braced forms require at least one hex digit between the braces.
    uint g1 = '\x{}';
//CHECK:      ^^^^^^ invalid string escape: '\x{}'

    uint h0 = '\u{';
//CHECK:      ^^^^^ invalid string escape: '\u{'

    uint h1 = '\u{}';
//CHECK:      ^^^^^^ invalid string escape: '\u{}'

    // A braced escape must be terminated with a closing brace.
    uint i = '\u{12';
//CHECK:     ^^^^^^^ invalid string escape: '\u{12'

    // A braced escape must be terminated with a closing brace.
    uint j = '\u{12z';
//CHECK:     ^^^^^^^^ invalid string escape: '\u{12'

    // A braced escape must be terminated with a closing brace.
    uint k = '\x{12z';
//CHECK:     ^^^^^^^^ invalid string escape: '\x{12'
}
