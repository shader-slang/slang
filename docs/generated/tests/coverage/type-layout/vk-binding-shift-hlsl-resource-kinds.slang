//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the -fvk-*-shift options make GLSL object layout allocate HLSL-style register kinds, so a Texture2D and a StructuredBuffer share the shifted t-range while sampler, cbuffer and UAV get their own.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Without a shift option, `GLSLObjectLayoutRulesImpl::GetObjectLayout` gives
// every object one descriptor-table slot and never consults
// `_getHLSLLayoutResourceKind`. Passing any `-fvk-<kind>-shift` sets
// `hlslToVulkanKindFlags`, which switches the rules over to allocating the
// *HLSL* resource kind for the shifted kinds, and that mapping table is what
// this test pins. The grouping is the interesting part: `Texture2D` and
// `StructuredBuffer` both map to `ShaderResource`, so they share the shifted
// `t` range (100 and 101) rather than each starting at their own base, while
// `SamplerState`, the `cbuffer` and the `RWStructuredBuffer` land on their own
// shifted bases.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute -fvk-t-shift 100 0 -fvk-s-shift 200 0 -fvk-b-shift 300 0 -fvk-u-shift 400 0

Texture2D tex;
SamplerState samp;
StructuredBuffer<float> sbuf;
RWStructuredBuffer<float4> outp;
cbuffer Cb { float scale; }
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = tex.SampleLevel(samp, float2(0, 0), 0) * scale * sbuf[0];
}

//SPV-DAG: OpDecorate %tex Binding 100
//SPV-DAG: OpDecorate %sbuf Binding 101
//SPV-DAG: OpDecorate %samp Binding 200
//SPV-DAG: OpDecorate %Cb Binding 300
//SPV-DAG: OpDecorate %outp Binding 400
