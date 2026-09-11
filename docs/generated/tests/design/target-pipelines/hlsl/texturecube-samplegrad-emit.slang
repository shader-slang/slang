//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
//META: purpose=Expansion: TextureCube `.SampleGrad(samp, dir, ddx, ddy)` survives HLSL emit with the SampleGrad spelling and the TextureCube native type.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SampleGrad on TextureCube takes a float3 direction plus two
// float3 gradient vectors. HLSL emit preserves all three
// constructor-style float3 arguments and the SampleGrad call.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

TextureCube<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 dir = float3(0.0, 0.0, 1.0);
    float3 dx = float3(0.01, 0.0, 0.0);
    float3 dy = float3(0.0, 0.01, 0.0);
    outBuf[tid.x] = tex.SampleGrad(samp, dir, dx, dy);
}

// CHECK: TextureCube{{.*}} tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: SampleGrad(
