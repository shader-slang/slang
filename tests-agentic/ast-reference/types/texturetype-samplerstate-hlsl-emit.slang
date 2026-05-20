//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0197e5bf25e2070b22239561996c86e1062d87133cd63b800da272aedb76f2aa
//META: purpose=A TextureType and SamplerStateType uniform are first-class types; the HLSL backend renders them as `Texture2D<T>` and `SamplerState`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `TextureType` is the `TextureTypeBase` leaf for HLSL-flavor
// sampled textures, and `SamplerStateType` is the SamplerState
// leaf. The doc notes both are "first-class types because each
// requires distinct IR lowering and binding semantics". Observed
// by emit-text inspection on the HLSL backend.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> g_tex;
SamplerState g_samp;
RWStructuredBuffer<float4> g_out;

//CHECK: Texture2D
//CHECK: SamplerState
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_tex.SampleLevel(g_samp, float2(0.0, 0.0), 0.0);
}
