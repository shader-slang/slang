//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Boundary: Texture2D sampled at the (0,0) origin corner emits with SampleLevel(samp, float2(0,0), 0) on HLSL.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: texture sample at the origin corner (0,0). The
// HLSL emit uses the canonical `SampleLevel` form with the
// separable Texture + SamplerState pair.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0.0, 0.0), 0.0);
}

// CHECK: SampleLevel(
// CHECK-SAME: float2(0.0
