//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
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

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint gi: SV_GroupIndex)
{
    // SPV: SubgroupId
    // HLSL: {{SV_GroupIndex|SV_GROUPINDEX}}
    outBuf[gi] = WaveGetWaveIndex();
}
