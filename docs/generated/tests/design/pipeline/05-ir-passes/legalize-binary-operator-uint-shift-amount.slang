//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=Expansion: legalize-binary-operator on WGSL handles a uint-typed shift count (`uint << uint`) — alternative operand-type axis to the int-shift case.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Operand-type variation on legalize-binary-operator: both operands
// of the `<<` are uint. WGSL requires the shift RHS to be `u32`;
// since the input is already uint the legalizer only needs to
// preserve the typing.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform uint a;
uniform uint b;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a << b;
}

// WGSL: u32
// WGSL: {{.*}}<<{{.*}}
