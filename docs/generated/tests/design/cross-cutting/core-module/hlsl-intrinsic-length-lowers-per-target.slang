//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=The HLSL meta-module's `length` intrinsic lowers per target — HLSL `length`, GLSL `length`, SPIR-V GLSL.std.450 `Length`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL meta-module exposes `length`. On SPIR-V it lowers
// through the GLSL.std.450 extended instruction set (OpExtInst
// ... Length). On HLSL and GLSL the spelling is `length`. We
// FileCheck each.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(float(tid.x), float(tid.y), float(tid.z));
    outBuf[tid.x] = length(v);
}

// HLSL: length(
// GLSL: length(
// SPIRV: Length
