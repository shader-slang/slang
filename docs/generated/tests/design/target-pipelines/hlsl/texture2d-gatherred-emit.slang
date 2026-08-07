//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Expansion: Texture2D `.GatherRed(samp, uv)` survives HLSL emit verbatim alongside the Texture2D / SamplerState pair.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// GatherRed reads the R channel of the 2x2 footprint texels. The
// HLSL emit preserves the method name and float2 coord.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.GatherRed(samp, float2(0.5, 0.5));
}

// CHECK: Texture2D{{.*}} tex_{{[0-9]+}} : register(t{{[0-9]+}})
// CHECK: GatherRed(
