//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=Target-specific lowering passes spell vector types in each language's idiom (HLSL `float3`, GLSL `vec3`, SPIR-V `OpTypeVector`, Metal `float3`, WGSL `vec3<f32>`, CUDA `float3`, CPP `Vector<float, 3>`).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `float3` is the same IR Vec(float, 3) type for every target; the
// per-target legalization passes (slang-ir-{hlsl,glsl,spirv,metal,
// wgsl,cuda}-legalize.cpp) emit it in the target's spelling. The
// CHECK patterns are deliberately loose for CUDA/CPP because the
// shared CLikeSourceEmitter renders `Vector<float, 3>` there.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float3(1.0, 2.0, 3.0);
}

// HLSL: float3
// GLSL: vec3
// SPIRV: OpTypeVector %float 3
// METAL: float3
// WGSL: vec3<f32>
// CUDA: float3
// CPP: Vector<float, 3>
