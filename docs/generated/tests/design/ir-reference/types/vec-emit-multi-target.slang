//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=5a7818957c25646b5df9f0a0213f5663d921d65b0099abf4ef8c860d93e0d5ae
//META: purpose=A float3 vector type lowers to target-specific spellings on each text-emit backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A float3 vector type lowers to target-specific spellings on each
// text-emit backend. The per-target text emit is divergent — HLSL
// keeps `floatN`, GLSL uses `vecN`, Metal keeps `floatN`, WGSL uses
// `vecN<f32>`, CPU C++ uses `Vector<float, N>`, and SPIR-V builds the
// type structurally as `OpTypeVector %float 3`. One directive per
// feasible target with a target-specific CHECK prefix exercises each
// spelling.

//TEST:SIMPLE(filecheck=CHECKHLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKGLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKMETAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKWGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKCPP):-target cpp -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKSPIRV):-target spirv-asm -stage compute -entry main

struct VecHolder { float3 v; }

uniform VecHolder data;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = data.v.x;
}

// CHECKHLSL: float3 v
// CHECKGLSL: vec3 v
// CHECKMETAL: float3 v
// CHECKWGSL: vec3<f32>
// CHECKCPP: Vector<float, 3>
// CHECKSPIRV: = OpTypeVector %float 3
