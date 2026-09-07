//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
//META: purpose=InterlockedAdd on a groupshared int lowers to OpAtomicIAdd against a Workgroup-storage pointer; validateAtomicOperations accepts atomics on Workgroup storage in addition to StorageBuffer.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: atomic operation on a non-default storage class.
// The Workgroup-storage variable holds the atomic counter; the
// SPIR-V backend emits OpAtomicIAdd against a Workgroup pointer.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

groupshared int counter;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
    if (gi == 0) counter = 0;
    GroupMemoryBarrierWithGroupSync();
    InterlockedAdd(counter, 1);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = counter;
}

// CHECK-DAG: %counter = OpVariable %_ptr_Workgroup_int Workgroup
// CHECK: OpAtomicIAdd %int %counter
