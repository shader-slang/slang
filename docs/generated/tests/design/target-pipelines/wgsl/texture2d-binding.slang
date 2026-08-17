//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=7d3641e4955ae039b13fce50516f0ed39854eeed3ca279f81a665ace48b92daa
//META: purpose=Texture2D<float4> emits as a global var with texture_2d<f32> type and a @binding/@group pair
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A Texture2D should map to WGSL's texture_2d<f32> type with a
// binding annotation.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// CHECK: @binding({{[0-9]+}}) @group(0) var tex_{{[0-9]+}} : texture_2d<f32>
// CHECK: sampler
