//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=53fa67e8e5b5b7cc0d09c737e54d406ba6f9a31c67e2efff85428bbfda70a405
//META: purpose=Verifies InterlockedAdd with uint MAX (0xFFFFFFFF) operand still lowers to atomicAdd on CUDA emit with the value 4294967295U.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: atomic-add with unsigned MAX operand. The pass
// `validateAtomicOperations` does not change behavior at the
// numeric boundary; the lowering still produces `atomicAdd` and
// the literal still survives as `4294967295U`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    InterlockedAdd(outBuf[0], uint(0xFFFFFFFFu));
}

// CHECK: atomicAdd
// CHECK-SAME: 4294967295U
