//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
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
