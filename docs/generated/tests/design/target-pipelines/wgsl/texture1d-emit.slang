//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
//META: purpose=Texture1D<float4> emits as a global var with WGSL texture_1d<f32> type with a @binding/@group annotation.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The WGSL emitter maps Texture1D<float4> to the WGSL
// `texture_1d<f32>` type (element scalar dropped to `f32`; the
// vec4 shape is implicit in the sampling-return type) with a
// per-resource binding pair.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

Texture1D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float(tid.x), 0).x;
}

// CHECK: var tex_{{[0-9]+}} : texture_1d<f32>
// CHECK: sampler
