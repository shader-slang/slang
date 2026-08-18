//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=The HLSL meta-module intrinsic length is emitted as the target's own length: HLSL/GLSL/Metal/WGSL length, the SPIR-V GLSL.std.450 Length extended instruction, and a generated helper on CUDA and C++.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `length` from the HLSL meta-module is accepted verbatim on
// every text-emit back end and lowered to that back end's own form.
// The vector is built from the dispatch id so the magnitude stays a
// runtime value; the SPIR-V prefix pins the extended-instruction form
// rather than a plain opcode, which is what distinguishes this
// intrinsic's lowering from `dot`.

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
    outBuf[tid.x] = length(a);
}

// HLSL: length(
// GLSL: length(
// SPIRV: OpExtInst %float %{{[A-Za-z0-9_]+}} Length
// METAL: length(
// WGSL: length(
// CUDA: length_{{[0-9]+}}(
// CPP: length_{{[0-9]+}}(
