//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
//META: purpose=Expansion (negative): operator-shift-overflow warns when shifting an `int8_t` by a count exceeding its 8-bit width — narrow-type axis to the existing int / uint cases.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Narrow-type variation on operator-shift-overflow. An `int8_t`
// shifted by 8 exceeds the 8-bit width and the pass diagnoses it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform int8_t a;
uniform RWStructuredBuffer<int8_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a << 8;
//CHECK:       ^^ left shift overflow
// CHECK: W41030
}
