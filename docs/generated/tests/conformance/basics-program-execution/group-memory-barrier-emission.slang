//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#thread-group-execution-model
//META: doc_section_digest=b513542addae4e13593e8fe9a226dda934c5e9723c7326c64dc0ad3ead5da188
//META: purpose=GroupMemoryBarrier() and GroupMemoryBarrierWithGroupSync() are thread-group barriers that emit the corresponding HLSL intrinsics and SPIR-V OpControlBarrier/OpMemoryBarrier ops.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "Related barriers include `GroupMemoryBarrier()` and
// `GroupMemoryBarrierWithGroupSync()`."
// We verify these two barriers are available in a compute entry point
// and emit recognizable patterns in HLSL (the literal function names)
// and in SPIR-V (ControlBarrier or MemoryBarrier).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

groupshared uint shared[2];

[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    shared[tid.x] = tid.x;
    GroupMemoryBarrier();
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = shared[1u - tid.x];
}

// HLSL: both barriers appear by name.
// CHECK-DAG: GroupMemoryBarrier
// CHECK-DAG: GroupMemoryBarrierWithGroupSync

// SPIR-V: barriers lower to OpControlBarrier or OpMemoryBarrier.
// CHECK-SPV: OpControlBarrier
