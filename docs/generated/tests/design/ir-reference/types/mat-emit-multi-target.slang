//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=fede71bd73c83957c94e27eab0113790d3ae8864e230db8dde234dca52b8d824
//META: purpose=A float4x4 matrix type lowers to target-specific spellings (float4x4 on HLSL, mat4x4 on GLSL, mat4x4<f32> on WGSL).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A float4x4 matrix type lowers to target-specific spellings (float4x4
// on HLSL, mat4x4 on GLSL, mat4x4<f32> on WGSL). Per-target text emit
// diverges: HLSL `float4x4`, GLSL `mat4x4`, WGSL uses `mat4x4<f32>`
// (or its `array<vec4<f32>, 4>` storage representation). Metal wraps
// the matrix in a storage struct (no portable single-token spelling),
// so it is not checked here.

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
