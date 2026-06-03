// Regression test for #11291 Bugs 1 and 2: well-formed octal and hex
// escapes in char and string literals must produce the byte value the
// user wrote.
//
//   - Bug 1 (`'\377'`) — `getCharLiteralValue` re-read a single-byte
//     `0xff` buffer through a UTF-8 decoder that walked off the end on
//     `0xff` (an invalid UTF-8 leading byte), returning random heap
//     bytes. Fixed by routing octal-escape values >= 0x80 through
//     `encodeUnicodePointToUTF8` in the escape handler so the buffer
//     always contains valid UTF-8, and bounding the reader at the
//     buffer end as defence-in-depth.
//
//   - Bug 2 (`"\xff"`) — `getStringLiteralTokenValue`'s inline hex
//     ladder forgot the `+ 10` for letter digits, mapping every
//     letter `a-f` / `A-F` to digit values 0-5 instead of 10-15.
//     `\xff` therefore decoded to `5*16+5 = 0x55 = 'U'`. Fixed by
//     replacing the inline ladder with `CharUtil::getHexDigitValue`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // Char-literal octal escape.
    let oct_max = '\377';
    //CHECK: oct_max=255
    printf("oct_max=%d\n", oct_max);

    // Char-literal hex escape, value < 0x80 (ASCII).
    let hex_ascii = '\x7e';
    //CHECK: hex_ascii=126
    printf("hex_ascii=%d\n", hex_ascii);

    // Char-literal hex escape, value >= 0x80 — exercises the
    // UTF-8-encode-then-decode round-trip.
    let hex_high = '\xff';
    //CHECK: hex_high=255
    printf("hex_high=%d\n", hex_high);

    // Char-literal hex escape, value > 0xFF but in Unicode range.
    let hex_unicode = '\x100';
    //CHECK: hex_unicode=256
    printf("hex_unicode=%d\n", hex_unicode);

    // Plain ASCII literal — unchanged baseline.
    let ascii = 'A';
    //CHECK: ascii=65
    printf("ascii=%d\n", ascii);
}
