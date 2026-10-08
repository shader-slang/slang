//TEST:SIMPLE:-target hlsl -stage compute -entry computeMain

// Regression test for https://github.com/shader-slang/slang/issues/5219
// The caret/underline in rich diagnostics must be positioned and sized by the
// terminal *display width* of the preceding source text, not by byte count or
// raw code-point count. Multi-byte (e.g. Greek) and wide (e.g. CJK) characters
// previously caused the underline to be misplaced and/or truncated to a single
// caret. The .expected baseline pins the exact caret columns.

void computeMain()
{
    // Two-byte UTF-8 code points (Greek). Three carets should land under `bbb`.
    /* αβγ */ bbb;

    // Wide (double-width) CJK code points; `救命` is four display columns.
    /* 救命 */ ddd;

    // A base letter followed by a zero-width combining mark (e + U+0301). The
    // combining mark must not advance the caret column, so the comment
    // `/* é */ ` is eight display columns just like ASCII `/* e */ `.
    /* é */ eee;

    // A tab is counted as a single display column (the renderer emits the
    // source line verbatim without expanding tabs, so this keeps the caret
    // arithmetic self-consistent rather than collapsing the tab to zero).
    /*	x */ fff;

    // The underlined token itself is wide: `救命` is two CJK code points
    // but four display columns, so the underline must be four carets (a byte
    // count would give six, a code-point count two).
    救命();
}
