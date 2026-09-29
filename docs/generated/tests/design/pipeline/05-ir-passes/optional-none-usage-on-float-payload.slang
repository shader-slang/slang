//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=667952fd5b9698b654c70c639e18194281145ed55e05baa506681d6bf7a7125d
//META: purpose=Expansion (negative): check-optional-none-usage rejects `.value` on an Optional<float> with float payload — payload-type axis to the existing int / explicit-none shapes.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Payload-type variation on check-optional-none-usage. An
// `Optional<float>` default-constructed (always none) is still
// flagged when its `.value` is read.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<float> o;
    buf[0] = o.value;
//CHECK:       ^^^^^ accessing .value on an Optional that is always none
// CHECK: E41027
}
