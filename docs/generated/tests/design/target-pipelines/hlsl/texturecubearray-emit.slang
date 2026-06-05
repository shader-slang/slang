//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Expansion: a TextureCubeArray resource keeps its HLSL spelling and lands in a `t` register on HLSL emit.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// TextureCubeArray uses a float4 sample coord (xyz = direction,
// w = array slice). HLSL emit preserves the native type name.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

TextureCubeArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float4(0.0, 1.0, 0.0, 0.0), 0.0);
}

// CHECK: TextureCubeArray{{.*}} tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: SampleLevel(
