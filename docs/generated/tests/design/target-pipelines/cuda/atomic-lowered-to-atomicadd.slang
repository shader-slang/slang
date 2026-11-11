//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=c5ba7e7159f13cfe3513cb6c3fd83b45567125805d4d70af544f0ece98ee1feb
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
