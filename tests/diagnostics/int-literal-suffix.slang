// Regression test for #11201: malformed integer-literal suffixes were
// silently accepted because the parser only counted L/U/Z occurrences
// without enforcing order or case-consistency.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void foo()
{
    // Well-formed suffixes (no diagnostics expected).
    let okA = 1L;
    let okB = 1U;
    let okC = 1UL;
    let okD = 1LU;
    let okE = 1LL;
    let okF = 1ll;
    let okG = 1ULL;
    let okH = 1LLU;
    let okI = 1Z;
    let okJ = 1uZ;
    let okK = 1Zu;

    // Malformed: 'Z' present but stray 'L's are silently dropped.
    let bad1 = 1ULLLLLLLZ;
//CHECK:       ^^^^^^^^^^ invalid suffix on integer literal
//CHECK:       ^^^^^^^^^^ invalid suffix 'ULLLLLLLZ' on integer literal

    // Malformed: 'U' interleaved between two 'L's; order matters.
    let bad2 = 1LUL;
//CHECK:       ^^^^ invalid suffix on integer literal
//CHECK:       ^^^^ invalid suffix 'LUL' on integer literal

    // Malformed: mixed case 'lL' (not a valid 'LL' or 'll').
    let bad3 = 1lL;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'lL' on integer literal

    // Malformed: mixed case the other way.
    let bad4 = 1Ll;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'Ll' on integer literal

    // Malformed: extra 'U' specifiers.
    let bad5 = 1UU;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'UU' on integer literal

    // Malformed: Z followed by L.
    let bad6 = 1ZL;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'ZL' on integer literal
}
