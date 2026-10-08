// Diagnostic test for a character literal whose value is out of range.
//
// This case is checked in its own file because it is diagnosed during
// parsing (by getCharLiteralValue), not during lexing. That diagnostic is
// suppressed once the file already contains lexer-phase errors, so it must
// be the only error present to be observed. The lexer-phase character
// literal errors live in char-literal-errors.slang.
//
// See docs/language-reference/expressions-literal.md for the literal format.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    // A '\x' escape value must fit in 32 bits; this one has nine hex digits
    // and overflows, so it is rejected as an invalid escape.
    uint a = '\x100000000';
    //CHECK: invalid string escape: '\x100000000'
}
