// Regression test for #11291 Bug 3: octal and hex escape sequences must
// diagnose values that exceed the legal range for their literal kind.
// Before the fix:
//   - `'\xffffff'` round-tripped through `encodeUnicodePointToUTF8` /
//     `getUnicodePointFromUTF8` and yielded a silently-wrong value (the
//     reporter saw `47`).
//   - `"\xffffff"` produced `"U"` because of the off-by-ten in the
//     inline hex ladder of `getStringLiteralTokenValue` (#11291 Bug 2)
//     and the missing overflow check.

//DIAGNOSTIC_TEST:SIMPLE:

void foo()
{
    // Char literal: hex escape exceeds Unicode max (0x10FFFF).
    let c1 = '\xffffff';

    // String literal: hex escape exceeds the byte range (0xFF).
    let s1 = "\xffffff";

    // String literal: octal escape exceeds the byte range (`\777` = 0x1FF).
    let s2 = "\777";

    // 9-digit hex literal: forces the running uint32_t accumulator to wrap
    // around before the final digit, so the post-loop value is < 0xFF (in
    // fact `0xff` here) but the `overflowed` guard must still fire. Catches
    // a future regression that drops the wraparound check.
    let s3 = "\x10000000ff";
}
