//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=The HLSL meta-module's `RWStructuredBuffer<T>` resource type legalizes to per-target binding shapes — HLSL `register(u..)`, GLSL/SPIR-V `Binding`, Metal `buffer(..)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWStructuredBuffer<T>` is one of the HLSL-named resource types
// the HLSL meta-module layers in. Each backend has a different
// binding marker; we FileCheck that the right shape appears.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: register(u{{[0-9]+}})
// GLSL: binding = {{[0-9]+}}
// SPIRV: OpDecorate {{.*}} Binding {{[0-9]+}}
// METAL: buffer({{[0-9]+}})
