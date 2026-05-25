// Regression test for #11278: an unterminated string literal must not cause
// the lexer's `getStringLiteralTokenValue` to read past the end of the
// token's content slice. The lexer emits a "newline in literal" diagnostic,
// but downstream parser sites (e.g. the StringLiteralExpr default-string
// path and `__getDecorator` name resolution) still call
// `getStringLiteralTokenValue` on the token's content; before the fix that
// loop kept reading until it hit a stray '"' byte in the surrounding heap.

//DIAGNOSTIC_TEST:SIMPLE:

// The following string literal is intentionally unterminated. The lexer
// reports it on the same source line, then the parser continues with a
// synthesized end-of-statement.
String s = "abc
;
