//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#wave-execution-model
//META: doc_section_digest=8d83b79ec2f7645747c3d299d4c9c52ee4ddc1279b8d96b52ef616d23d94159e
//META: purpose=WaveActiveBallot is a wave-tangled ballot function available in compute entry points; it emits WaveActiveBallot in HLSL and OpGroupNonUniformBallot in SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "Threads in a wave can synchronize and share data efficiently
// using wave-tangled functions such as ballots, reductions, shuffling, control
// flow barriers with the wave scope, and similar operations."
//
// WaveActiveBallot is the canonical ballot intrinsic. We verify it compiles
// and emits the target-appropriate representation.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint4 ballot = WaveActiveBallot(true);
    output[tid.x] = ballot.x;
}

// HLSL emits WaveActiveBallot intrinsic.
// CHECK: WaveActiveBallot

// SPIR-V emits GroupNonUniformBallot (the Vulkan ballot extension opcode).
// CHECK-SPV: GroupNonUniformBallot
