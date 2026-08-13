//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Negative boundary: integer `a / 0` is rejected by semantic check with "divide by zero" — no `div` IR opcode is produced. (Float divide-by-zero is permitted; see `div-float-by-positive-zero-uniform.slang`.)
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative edge of the `div` claim: `/` normally produces
// `div`, but on integers, dividing by the zero literal is
// undefined behavior and Slang rejects it at semantic check
// rather than lowering it to a `div` opcode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a / 0;
//CHECK:       ^ divide by zero
// CHECK: E30002
}
