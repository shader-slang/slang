//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=908553c64f782960d2c5717a98c49179101f5061838a971f9566e0daf48453d4
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
