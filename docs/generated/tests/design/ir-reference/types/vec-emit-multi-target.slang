//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=3375fc96f25274ba6b5f61268aab7b799044240063cd3b64ac3928c13a5702fc
//META: purpose=A float3 vector type lowers to target-specific spellings on each text-emit backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `Vec` is the IR encoding of HLSL `vector<T,N>`. The
// per-target text emit is divergent — HLSL keeps `floatN`, GLSL
// uses `vecN`, Metal keeps `floatN`, WGSL uses `vecN<f32>`, CPU C++
// uses `Vector<float, N>`. One directive per feasible target with
// a target-specific CHECK prefix exercises each spelling.

//TEST:SIMPLE(filecheck=CHECKHLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKGLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKMETAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKWGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECKCPP):-target cpp -stage compute -entry main

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
