//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
//META: purpose=Texture2DArray<float4> emits as a global var with WGSL texture_2d_array<f32> type and a @binding/@group annotation.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The WGSL emitter maps Texture2DArray<float4> to the WGSL
// `texture_2d_array<f32>` type. A `float3(coord, layer)`
// Sample lowers to `textureSampleLevel(tex, samp, xy, i32(layer), 0)`
// (the .xy / layer split is performed at emit time).

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture2DArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(tid), 0).x;
}

// CHECK: var tex_{{[0-9]+}} : texture_2d_array<f32>
// CHECK: textureSampleLevel
