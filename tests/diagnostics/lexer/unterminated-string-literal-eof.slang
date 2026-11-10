// Regression test for #11278: an unterminated string literal that runs
// off the end of the source file (no trailing newline) drives the cursor
// to `end` while inside the post-backslash escape leadin guard at line
// 1138 of `getStringLiteralTokenValue`. The lexer reports
// `endOfFileInLiteral` and the consumer must return its accumulated
// `StringBuilder` rather than reading past the token slice.
//
// The file intentionally ends mid-literal — there is no terminating
// newline after the trailing `\\`. Do not let an editor add one.

//DIAGNOSTIC_TEST:SIMPLE:

String s = "abc\