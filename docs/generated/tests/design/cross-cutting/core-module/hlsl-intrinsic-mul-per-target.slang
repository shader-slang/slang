//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=The HLSL meta-module intrinsic mul keeps its name on HLSL, becomes the * operator on GLSL/Metal/WGSL, becomes OpVectorTimesMatrix on SPIR-V and a generated helper on CUDA and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `mul` is an HLSL-compatibility name rather than a universal
// spelling: only HLSL keeps the call, while the other back ends
// re-express the same core-module declaration in their own algebra.
// The matrix and vector are both built from dispatch ids so the
// product survives folding, and each prefix CHECKs that back end's
// form of the multiply.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m = float4x4(float(tid.x));
    float4 v = float4(1.0, 2.0, 3.0, float(tid.y));
    outBuf[tid.x] = mul(m, v).x;
}

// HLSL: mul(
// GLSL: * (mat4x4(
// SPIRV: OpVectorTimesMatrix %v4float
// METAL: * (matrix<float,int(4),int(4)>
// WGSL: * (mat4x4<f32>(
// CUDA: mul_{{[0-9]+}}(
// CPP: mul_{{[0-9]+}}(
