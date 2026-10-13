//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=A matrix layout modifier such as `row_major` applied to a non-matrix type is rejected (E39026), and the alias still resolves to the plain type.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the matrix-layout branch of visitModifiedTypeExpr, which is
// handled before the general checkTypeModifier ladder: a layout modifier is only
// turned into a layout `Val` when the base type is a matrix, and otherwise it is
// diagnosed and dropped. Dropping it (rather than producing an error type) is
// what lets the alias below still be used as an ordinary `int` without a
// follow-on error, which is the behaviour this test pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

typedef row_major int RowMajorInt;
//CHECK:^^^^^^^^^ E39026
//CHECK:^^^^^^^^^ matrix layout modifier cannot be used on non-matrix type 'int'.

int test()
{
    RowMajorInt x = 1;
    return x;
}
