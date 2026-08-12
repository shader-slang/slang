//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
//META: purpose=Texture2D<float4>.Sample(samp, uv) lowers to WGSL textureSample(tex, samp, uv) without an explicit LOD argument.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2D.Sample(samp, uv)` uses pixel-shader derivatives.
// In compute it still lowers to WGSL's `textureSample(...)`
// without an explicit LOD argument (contrast with SampleLevel
// which lowers to textureSampleLevel and an explicit LOD).

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Sample(samp, float2(tid.xy));
}

// CHECK: texture_2d<f32>
// CHECK: textureSample
