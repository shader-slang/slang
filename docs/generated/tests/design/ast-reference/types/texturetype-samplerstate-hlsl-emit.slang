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

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

Texture2D<float4> g_tex;
SamplerState g_samp;
RWStructuredBuffer<float4> g_out;

//CHECK-DAG: Texture2D
//CHECK-DAG: SamplerState
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_tex.SampleLevel(g_samp, float2(0.5, 0.5), 0.0);
}
