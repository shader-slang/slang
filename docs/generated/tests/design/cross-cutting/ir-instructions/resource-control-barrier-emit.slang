//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=9e5354e38bb09fc16bac90544687941aec8ef947b56cd9f9b65ce3ddb041f315
//META: purpose=`GroupMemoryBarrierWithGroupSync()` is a nullary barrier opcode that emits as each target's group-wide execution barrier.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `ControlBarrier / GroupMemoryBarrierWithGroupSync / ...`
// barrier row, whose operand column is empty — the opcodes are nullary, so
// the only observable is the emitted barrier call itself. A groupshared
// array is written, the barrier is issued, and a *different* lane's slot is
// read back, so the barrier is load-bearing and cannot be dropped. Each
// CHECK asserts the target's own barrier spelling; the `cpp` kernel target
// is excluded because it has no thread-group execution model to synchronize.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

groupshared uint sdata[4];
uniform RWStructuredBuffer<uint> outbuf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
    sdata[gi] = gi;
    GroupMemoryBarrierWithGroupSync();
    outbuf[gi] = sdata[3 - gi];
}

// HLSL: GroupMemoryBarrierWithGroupSync();
// GLSL: controlBarrier(
// SPV: OpControlBarrier
// METAL: threadgroup_barrier(
// WGSL: workgroupBarrier();
// CUDA: __syncthreads();
