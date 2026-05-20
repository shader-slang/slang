//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=A float4x4 matrix type lowers to target-specific spellings (float4x4 on HLSL, mat4x4 on GLSL, mat4x4<f32> on WGSL).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Mat` is the IR encoding of a fixed-shape matrix.
// Per-target text emit diverges: HLSL `float4x4`, GLSL `mat4x4`,
// WGSL uses `mat4x4<f32>` (or its `array<vec4<f32>, 4>` storage
// representation). Metal wraps the matrix in a storage struct (no
// portable single-token spelling), so it is not checked here.

//TEST:SIMPLE(filecheck=CHECKHLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKGLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKWGSL):-target wgsl -stage compute -entry main

uniform float4x4 m;
uniform float4 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (mul(m, v)).x;
}

// CHECKHLSL: float4x4 m
// CHECKGLSL: mat4x4 m
// CHECKWGSL: mat4x4<f32>
