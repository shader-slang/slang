//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9bd773e205e5aa520ee4875c2a6aebc4a9d9aabba8cfac9d3b66489e9488c043
//META: purpose=A Slang `float3` parameter is emitted with the target-native vector spelling: HLSL/Metal `float3`, GLSL `vec3`, WGSL `vec3<f32>`, CUDA `float3 `, CPP `Vector<float, 3>`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Each C-like backend has its own vector type spelling. The shared
// C-like-base lets each target subclass override the type printer
// only where the targets disagree, and the disagreement is
// concentrated in the vector / matrix names.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

uniform float3 inV;
RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inV * 2.0;
}

// HLSL: float3 inV_{{[0-9]+}}
// GLSL: vec3 inV_{{[0-9]+}}
// METAL: float3 inV_{{[0-9]+}}
// WGSL: inV_{{[0-9]+}} : vec3<f32>
// CUDA: float3{{ +}}inV_{{[0-9]+}}
// CPP: Vector<float, 3>{{ +}}inV_{{[0-9]+}}
