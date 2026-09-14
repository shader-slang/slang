//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=Texture2D, RWTexture2D and SamplerState from the HLSL meta-module lower to each target's own texture and sampler vocabulary.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the HLSL-named texture and sampler types are supplied by
// the HLSL meta-module for every back end, not just for HLSL output.
// One shader samples a Texture2D through a SamplerState and stores
// into an RWTexture2D; each prefix CHECKs that target's own texture,
// sampler and image-store spelling, which shows the same declarations
// being re-expressed per target rather than passed through.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

Texture2D<float4> tex;
SamplerState samp;
RWTexture2D<float4> rwTex;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4 c = tex.SampleLevel(samp, float2(float(tid.x), float(tid.y)), 0.0);
    rwTex[tid.xy] = c;
    outBuf[tid.x] = c.x;
}

// HLSL-DAG: Texture2D<float4 > tex_{{[0-9]+}} : register(t
// HLSL-DAG: SamplerState samp_{{[0-9]+}} : register(s
// HLSL-DAG: RWTexture2D<float4 > rwTex_{{[0-9]+}} : register(u
// GLSL-DAG: uniform texture2D tex_{{[0-9]+}}
// GLSL-DAG: uniform sampler samp_{{[0-9]+}}
// GLSL-DAG: uniform image2D rwTex_{{[0-9]+}}
// SPIRV-DAG: OpTypeImage %float 2D
// SPIRV-DAG: OpTypeSampler
// SPIRV-DAG: OpImageSampleExplicitLod
// SPIRV-DAG: OpImageWrite
// METAL-DAG: texture2d<float, access::sample> tex_{{[0-9]+}}
// METAL-DAG: sampler samp_{{[0-9]+}}
// METAL-DAG: texture2d<float, access::read_write> rwTex_{{[0-9]+}}
// WGSL-DAG: texture_2d<f32>
// WGSL-DAG: sampler
// WGSL-DAG: textureStore(
// CUDA-DAG: CUtexObject tex_{{[0-9]+}}
// CUDA-DAG: CUsurfObject rwTex_{{[0-9]+}}
// CUDA-DAG: surf2Dwrite<
// CPP-DAG: Texture2D<Vector<float, 4> > tex_{{[0-9]+}}
// CPP-DAG: SamplerState samp_{{[0-9]+}}
// CPP-DAG: RWTexture2D<Vector<float, 4> > rwTex_{{[0-9]+}}
