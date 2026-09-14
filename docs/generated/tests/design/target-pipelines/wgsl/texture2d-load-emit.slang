//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=7d3641e4955ae039b13fce50516f0ed39854eeed3ca279f81a665ace48b92daa
//META: purpose=Texture2D<float4>.Load(int3(xy, lod)) lowers to WGSL textureLoad(tex, xy, lod) (no sampler involved).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2D.Load(int3(xy, lod))` is the HLSL-style unfiltered
// texel-fetch. WGSL spells it `textureLoad(tex, xy, lod)` — the
// emitter splits the int3 argument into the 2D coord and the LOD
// component automatically.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture2D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int3(int2(tid.xy), 0));
}

// CHECK: texture_2d<f32>
// CHECK: textureLoad
