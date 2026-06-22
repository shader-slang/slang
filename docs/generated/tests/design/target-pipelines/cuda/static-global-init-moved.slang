//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#inlineglobalconstantsforlegalization-for-cuda
//META: doc_section_digest=fa0d5c3ce0756b2f99f26c93a59640e701e9d459ee33289b2cf6e6474ae6405a
//META: purpose=CUDA always runs inlineGlobalConstantsForLegalization so a static module-scope constant is inlined at its use sites as the literal value because nvrtc rejects dynamic __device__ initialization.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `inlineGlobalConstantsForLegalization` has an explicit CUDA short
// circuit (`target == CUDASource ||
// (isCPUTarget && isKernelTarget) || ...`) at line ~1543 because
// nvrtc rejects dynamic `__device__` initialization. The static
// constant value is inlined at the use site rather than emitted
// as a `__device__` global with an initializer.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

static const int counter = 5;

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = counter + int(tid.x);
}

// CHECK: extern "C" __global__ void main
// CHECK: int(5)
// CHECK-NOT: __device__ int counter
