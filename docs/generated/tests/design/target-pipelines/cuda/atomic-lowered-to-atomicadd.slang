//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=53fa67e8e5b5b7cc0d09c737e54d406ba6f9a31c67e2efff85428bbfda70a405
//META: purpose=An atomic operation on a UAV passes validateAtomicOperations and emits as the CUDA atomicAdd intrinsic in the CUDA C++ text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `validateAtomicOperations` runs in Phase C with
// `target != SPIRV` and CUDA passes through; the C-like emitter then
// writes the operation as the CUDA `atomicAdd` intrinsic instead of
// the HLSL `InterlockedAdd` shape.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    InterlockedAdd(outBuf[0], int(tid.x));
}

// CHECK: atomicAdd
