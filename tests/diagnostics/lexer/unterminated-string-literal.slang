// Regression test for #11278: an unterminated string literal must not cause
// the lexer's `getStringLiteralTokenValue` to read past the end of the
// token's content slice. The fix added bound checks at five separate sites
// inside `getStringLiteralTokenValue`; each test below drives the cursor to
// `end` while inside one of the new guards, so a future revert of any one
// of them is caught.
//
// These cases assert only that the lexer emits its diagnostic and the
// parser does not crash. A silent garbage-in-value regression (the consumer
// reads past `end` and returns junk that happens not to crash) cannot be
// caught from diagnostic-only output without ASAN/MSAN; the value-pinning
// path through `-dump-ast` is a follow-up.

//DIAGNOSTIC_TEST:SIMPLE:

// (1) Top-of-loop bound check: `while (cursor < end)` exits before
// trying to read the missing closing quote.
String a = "abc
;

// (2) Octal sub-loop bound check: `\1` is the start of an octal escape;
// the inner sub-loop must stop before reading the byte that would have
// followed.
String b = "abc\1
;

// (3) Hex sub-loop bound check: same as (2) but for `\x`.
String c = "abc\x
;
