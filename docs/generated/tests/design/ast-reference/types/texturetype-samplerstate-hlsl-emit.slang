//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=A TextureType `Texture2D<T>` and a SamplerStateType `SamplerState` are separate resource types accepted as uniforms, and both spellings survive HLSL emission.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the sampled-texture and sampler-state families are
// distinct types that combine in a sample call: the sampler is passed
// as an argument to the texture's SampleLevel rather than being part of
// the texture type. CHECK matches both spellings in the emitted HLSL.
// Every other text-emit back-end keeps the same split into two separate
// resources that meet at the sample call: two distinct uniforms in
// GLSL joined by `sampler2D(...)`, two separate kernel arguments in
// Metal, an image variable plus a sampler variable combined by
// `OpSampledImage` in SPIR-V, and two `var` declarations passed to
// `textureSampleLevel` in WGSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

Texture2D<float4> g_tex;
SamplerState g_samp;
RWStructuredBuffer<float4> g_out;

//CHECK-DAG: Texture2D
//CHECK-DAG: SamplerState

//GLSL-DAG: uniform texture2D g_tex_0;
//GLSL-DAG: uniform sampler g_samp_0;
//GLSL-DAG: sampler2D(g_tex_0,g_samp_0)

//METAL-DAG: texture2d<float, access::sample> g_tex_{{[0-9]+}} {{\[\[}}texture(0)
//METAL-DAG: sampler g_samp_{{[0-9]+}} {{\[\[}}sampler(0)
//METAL-DAG: (g_tex_{{[0-9]+}}).sample((g_samp_{{[0-9]+}})

//SPIRV-DAG: OpTypeImage %float 2D 2 0 0 1 Unknown
//SPIRV-DAG: OpTypeSampler
//SPIRV-DAG: OpSampledImage

//WGSL-DAG: var g_tex_0 : texture_2d<f32>
//WGSL-DAG: var g_samp_0 : sampler
//WGSL-DAG: textureSampleLevel((g_tex_0), (g_samp_0)
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_tex.SampleLevel(g_samp, float2(0.5, 0.5), 0.0);
}
