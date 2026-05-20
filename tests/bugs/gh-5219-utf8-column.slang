// Regression test for GH-5219: error underline misaligned for multi-byte UTF-8.
//
// The compiler must report code-point columns (not byte offsets) for diagnostics.
// The rich diagnostic renderer must also use the code-point column when computing
// the byte slice for the lexer (to determine the highlight span length).
//
// In each case below, the comment before `undeclaredN` has exactly 9 code points
// (matching `/* abc */`), so all three identifiers sit at code-point column 20.
// `undeclaredN` is 11 ASCII characters, so the span ends at code-point column 31.
// Without the fix, byte-based slicing would return a short/wrong token for a2/a3.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Case 1: ASCII (1-byte code points) — code-point column == byte column.
int a1 = /* abc */ undeclared1;
//CHECK:           ^^^^^^^^^^^ undefined identifier

// Case 2: Two-byte UTF-8 code points (α β γ, 2 bytes each).
int a2 = /* αβγ */ undeclared2;
//CHECK:           ^^^^^^^^^^^ undefined identifier

// Case 3: Three-byte UTF-8 code points (CJK characters, 3 bytes each).
int a3 = /* 救命！ */ undeclared3;
//CHECK:           ^^^^^^^^^^^ undefined identifier

// Case 4: Four-byte UTF-8 code points (supplementary plane, 4 bytes each).
int a4 = /* 𝛼𝛽𝛾 */ undeclared4;
//CHECK:           ^^^^^^^^^^^ undefined identifier
