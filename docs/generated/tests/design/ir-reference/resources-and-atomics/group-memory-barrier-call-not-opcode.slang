//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#controlbarrier-vs-groupmemorybarrierwithgroupsync
//META: doc_section_digest=1ff69c327de4ae40216584981ae7517b53fd5dc7120baac3f27ac252e1508864
//META: purpose=`GroupMemoryBarrierWithGroupSync()` never creates the opcode of the same name — the core-module function is `__intrinsic_asm` per target, so lowering leaves a `call` and each backend emits its own barrier text.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "no producer" claim from both ends. The IR
// directive pins `call %GroupMemoryBarrierWithGroupSync()` inside
// `func %main`, showing that lowering does not synthesize an opcode; the
// per-target directives then pin the target-specific text each
// `__intrinsic_asm` arm produces, which is only possible if the barrier
// travels to emit as a call rather than as a shared opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform RWStructuredBuffer<uint> rwbuf;
groupshared uint g[4];

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g[tid.x] = tid.x;
    GroupMemoryBarrierWithGroupSync();
    rwbuf[tid.x] = g[3 - tid.x];
}

// IR-LABEL: func %main
// IR: call %GroupMemoryBarrierWithGroupSync()

// HLSL: GroupMemoryBarrierWithGroupSync()
// GLSL: controlBarrier(
// SPV: OpControlBarrier
// METAL: threadgroup_barrier(
// CUDA: __syncthreads()
