//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#wave-intrinsics
//META: doc_section_digest=1e0b46909a936eac74d813141d3ebfecac75a795c30e5a516e30850675096d2a
//META: purpose=`WaveGetActiveMask()` does not reach `func %main` as the `waveGetActiveMask` opcode — lowering leaves `call %WaveGetActiveMask()` and each backend emits its own active-mask expression.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies where the wave *mask* opcodes actually sit relative to user
// code. The intrinsic's result is stored to a buffer so it cannot be
// dead, and the IR directive pins the call form under `func %main`; the
// HLSL, GLSL, SPIR-V, Metal, and CUDA directives then pin each backend's
// own ballot expression, which is what the call resolves to at emit.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    rwbuf[tid.x] = WaveGetActiveMask();
}

// IR-LABEL: func %main
// IR: call %WaveGetActiveMask()

// HLSL: WaveActiveBallot(
// GLSL: subgroupBallot(
// SPV: OpGroupNonUniformBallot
// METAL: simd_ballot(
// CUDA: __ballot_sync(
