//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
//META: purpose=Boundary: a SamplerComparisonState bound `register(s0)` (lower-edge sampler index) preserves its explicit `s0` annotation in HLSL emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: lowest legal `s` register index (0) on a
// SamplerComparisonState. HLSL emit carries the explicit
// register annotation through verbatim.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float> tex;
SamplerComparisonState sampCmp : register(s0);
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleCmp(sampCmp, float2(0.5, 0.5), 0.0);
}

// CHECK: SamplerComparisonState sampCmp_{{[0-9]+}} : register(s0)
