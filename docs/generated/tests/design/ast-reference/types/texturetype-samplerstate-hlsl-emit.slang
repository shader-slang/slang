//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=A TextureType Texture2D<T> and a SamplerStateType SamplerState are accepted as uniforms and a sample dispatches; the HLSL emit renders both spellings.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies TextureType and SamplerStateType are accepted at uniform
// parameters and a Sample call type-checks. The HLSL backend renders
// the Texture2D and SamplerState spellings. CHECK matches both in the
// emitted HLSL.

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
