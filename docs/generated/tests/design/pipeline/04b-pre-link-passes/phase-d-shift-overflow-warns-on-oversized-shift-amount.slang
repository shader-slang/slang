//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=Phase D's checkForOperatorShiftOverflow (D7) flags a left-shift whose constant shift count exceeds the width of the shifted operand.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shift-width validator at the boundary it cares about: a
// 32-bit operand shifted by 40. The shifted value comes from a `uniform`
// so the expression is not constant-folded out from under the checker
// before it runs. The code CHECK pins the warning identity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a << 40;
//CHECK:       ^^ left shift overflow
//CHECK:       ^^ left shift amount exceeds the number of bits and the result will be always zero, (`int' << `40`).
// CHECK: W41030
}
