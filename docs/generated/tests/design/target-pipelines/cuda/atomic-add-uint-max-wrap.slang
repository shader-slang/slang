//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=60a878125219aee6cefce03caff6a6f1ca88b56b8f0f8f9f00a510f58c6ec5ad
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
