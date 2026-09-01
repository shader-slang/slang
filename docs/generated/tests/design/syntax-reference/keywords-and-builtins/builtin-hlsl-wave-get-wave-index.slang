//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
//META: purpose=`hlsl.meta.slang` layers in the wave-index builtins: `WaveGetWaveIndex` resolves in a compute shader and lowers to a load of the SubgroupId builtin in SPIR-V, and an `SV_GroupIndex` compute entry parameter survives into the emitted HLSL signature.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two names from the hlsl.meta.slang wave-builtin bullet are
// bound to their documented roles. The entry takes an SV_GroupIndex
// parameter and stores WaveGetWaveIndex() through it so DCE keeps
// both. SPV asserts the SPIR-V loads the SubgroupId builtin (only
// possible if WaveGetWaveIndex resolved to the wave-index intrinsic);
// HLSL asserts the semantic spelling survives into the entry
// signature (the emitter may upper-case semantics, so both spellings
// are accepted).

// The remaining shader text targets each resolve the same intrinsic to
// their own wave-index builtin, so the claim fans out to all five:
// GLSL to `gl_SubgroupID` (pulling in `GL_KHR_shader_subgroup_basic`),
// Metal to the `simdgroup_index_in_threadgroup` entry-point builtin,
// and WGSL to `@builtin(subgroup_id)` under `enable subgroups`.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint gi: SV_GroupIndex)
{
    // SPV: SubgroupId
    // HLSL: {{SV_GroupIndex|SV_GROUPINDEX}}
    outBuf[gi] = WaveGetWaveIndex();
}

//GLSL-DAG: GL_KHR_shader_subgroup_basic
//GLSL-DAG: gl_SubgroupID
//GLSL-DAG: gl_LocalInvocationIndex

//METAL: {{\[\[}}simdgroup_index_in_threadgroup

//WGSL-DAG: enable subgroups
//WGSL-DAG: @builtin(subgroup_id)
//WGSL-DAG: @builtin(local_invocation_index)
