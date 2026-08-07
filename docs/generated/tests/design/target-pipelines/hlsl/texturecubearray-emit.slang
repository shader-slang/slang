//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
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
