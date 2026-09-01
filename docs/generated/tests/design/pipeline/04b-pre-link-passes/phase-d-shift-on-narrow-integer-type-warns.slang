//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=Phase D's checkForOperatorShiftOverflow also covers the complementary narrow-operand case, warning W41034 when a sub-32-bit left operand is shifted by a wider non-literal amount.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second diagnostic the shift validator owns. Its sibling
// covers the oversized-constant-amount case; this one covers the branch
// where the amount is not a literal and the left operand is narrower than
// 32 bits, which is where Slang differs from C by not promoting the left
// operand first. Both operands come from `uniform`s so the expression is
// still an Lsh when the validator runs. The caret pins the warning to the
// shift operator and the annotated text names the operand type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int16_t s;
uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (int)(s << n);
//CHECK:             ^^ left shift on narrow integer type
//CHECK:             ^^ left shift on narrow integer type 'int16_t'
// CHECK: W41034
}
