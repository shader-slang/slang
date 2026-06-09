//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
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
