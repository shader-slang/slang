//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Expansion: a Texture2DArray resource keeps its HLSL spelling and lands in a `t` register; SampleLevel takes a float3 coord (uv + slice).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Texture2DArray uses a float3 sample coord (uv + array slice).
// HLSL emit preserves the native type name and call shape.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2DArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(0.5, 0.5, 0.0), 0.0);
}

// CHECK: Texture2DArray{{.*}} tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: SampleLevel(
