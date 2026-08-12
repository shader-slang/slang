//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=8489fad23ce3c21f6f3d24794bba24b5d2a69ab1e82e6e16bfa45871b21462f3
//META: purpose=The HLSL meta-module intrinsic dot is emitted as the target's own dot: HLSL/GLSL/Metal/WGSL dot, SPIR-V OpDot, and a generated dot_ helper on CUDA and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the HLSL meta-module makes `dot` compile unchanged on every
// text-emit back end and that each back end supplies its own spelling.
// Both operands are derived from the dispatch id so the product cannot
// be constant-folded; each prefix CHECKs the call form that back end
// emits, including the SPIR-V opcode and the generated helper C-family
// targets fall back to.

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
    float3 a = float3(float(tid.x), 1.0, 2.0);
    float3 b = float3(3.0, float(tid.y), 5.0);
    outBuf[tid.x] = dot(a, b);
}

// HLSL: dot(
// GLSL: dot(
// SPIRV: OpDot %float
// METAL: dot(
// WGSL: dot(
// CUDA: dot_{{[0-9]+}}(
// CPP: dot_{{[0-9]+}}(
