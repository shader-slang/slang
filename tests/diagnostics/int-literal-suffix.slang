// Regression test for #11201: malformed integer-literal suffixes were
// silently accepted because the parser only counted L/U/Z occurrences
// without enforcing order or case-consistency.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void foo()
{
    // Well-formed suffixes (no diagnostics expected). Includes lowercase
    // and mixed unsigned/length case (`uL`, `Lu`, `lU`, `Ul` are all
    // valid because the case rule applies *within* the length specifier
    // only — the unsigned specifier and the length specifier may differ
    // in case).
    let okA = 1L;
    let okB = 1l;
    let okC = 1U;
    let okD = 1u;
    let okE = 1Z;
    let okF = 1z;
    let okG = 1UL;
    let okH = 1uL;
    let okI = 1Lu;
    let okJ = 1lU;
    let okK = 1Ul;
    let okL = 1LU;
    let okM = 1LL;
    let okN = 1ll;
    let okO = 1ULL;
    let okP = 1LLU;
    let okQ = 1uZ;
    let okR = 1Zu;

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

    // Malformed: Z followed by L (length specifier already consumed).
    let bad6 = 1ZL;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'ZL' on integer literal

    // Malformed: L followed by Z.
    let bad7 = 1LZ;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'LZ' on integer literal

    // Malformed: doubled Z.
    let bad8 = 1ZZ;
//CHECK:       ^^^ invalid suffix on integer literal
//CHECK:       ^^^ invalid suffix 'ZZ' on integer literal

    // Malformed: U on both sides of length specifier.
    let bad9 = 1ULU;
//CHECK:       ^^^^ invalid suffix on integer literal
//CHECK:       ^^^^ invalid suffix 'ULU' on integer literal

    // Malformed: U on both sides of Z.
    let bad10 = 1UZU;
//CHECK:        ^^^^ invalid suffix on integer literal
//CHECK:        ^^^^ invalid suffix 'UZU' on integer literal
}
