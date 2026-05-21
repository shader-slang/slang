//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=61134c3253681818e651a9810123d97ae5bcc929737f8bed5d159bd896cf3248
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
}
