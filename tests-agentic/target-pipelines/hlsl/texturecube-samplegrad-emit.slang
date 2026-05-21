//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
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
