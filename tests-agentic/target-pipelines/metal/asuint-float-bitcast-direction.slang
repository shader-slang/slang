//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=Boundary: chained `asfloat(asuint(...))` round-trip emits nested `as_type<uint>(as_type<float>(...))` wrappers — Slang preserves both directions through Metal lowering.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: bitcast roundtrip. We chain
// `asuint(asfloat(asuint(v)))` and verify both `as_type<uint>`
// and `as_type<float>` survive in the emitted Metal text.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<uint> outBuf;
uniform float v;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float roundtrip = asfloat(asuint(v));
    outBuf[tid.x] = asuint(roundtrip) + tid.x;
}

// CHECK: as_type<uint>
// CHECK: as_type<float>
