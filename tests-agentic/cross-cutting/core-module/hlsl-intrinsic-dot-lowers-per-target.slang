//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=The HLSL meta-module's `dot` intrinsic lowers to per-target spellings: HLSL `dot`, GLSL `dot`, SPIR-V `OpDot`, CUDA scalar fma chain.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL meta-module exposes the `dot` intrinsic carrying
// `__target_intrinsic` modifiers for each backend. We exercise `dot`
// on a vector and FileCheck the per-target spelling.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 a = float3(float(tid.x), float(tid.y), float(tid.z));
    float3 b = float3(1.0, 2.0, 3.0);
    outBuf[tid.x] = dot(a, b);
}

// HLSL: dot(
// GLSL: dot(
// SPIRV: OpDot
