//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=Boundary: asfloat(uint) renders as as_type<float>(...) on Metal (bitcast direction uint->float, dual of asuint).
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: as_type<T> bitcast directions. The companion
// smoke test (`bitcast-emits-as-type.slang`) probes asuint(float).
// Here we probe the reverse direction: asfloat(uint) must also
// render as `as_type<float>(...)`. Source bits come from
// SV_DispatchThreadID so DCE cannot fold away the bitcast.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = asfloat(tid.x);
}

// CHECK: as_type<float>
