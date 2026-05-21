//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=Expansion: legalize-matrix-types lowers a Slang `matrix<float, 3, 4>` into a target-shaped matrix on HLSL (`float3x4`) and GLSL (`mat3x4` / `mat4x3`-style spelling).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Legalize matrix types" row of the type-and-value legalization
// table (`slang-ir-legalize-matrix-types.cpp`) renders a `matrix<T,
// R, C>` into the target's native matrix shape. HLSL spells this
// `float3x4`; GLSL emits a `mat`-prefixed spelling.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    matrix<float, 3, 4> m;
    m[0] = float4(a, a + 1.0f, a + 2.0f, a + 3.0f);
    m[1] = float4(a + 4.0f, a + 5.0f, a + 6.0f, a + 7.0f);
    m[2] = float4(a + 8.0f, a + 9.0f, a + 10.0f, a + 11.0f);
    buf[0] = m[1][2];
}

// HLSL: float3x4
// GLSL: mat
