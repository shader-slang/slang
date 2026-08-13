//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Expansion: Texture2D `.SampleGrad(samp, uv, ddx, ddy)` survives HLSL emit with the float2 coord and gradient pair.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SampleGrad on Texture2D takes float2 UV and two float2
// gradient vectors. HLSL emit preserves both the call spelling
// and the constructor-style float2 literals.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(0.5, 0.5);
    float2 dx = float2(0.01, 0.0);
    float2 dy = float2(0.0, 0.01);
    outBuf[tid.x] = tex.SampleGrad(samp, uv, dx, dy);
}

// CHECK: Texture2D{{.*}} tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: SampleGrad(
