//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=A static index of N-1 into a fixed-size array passes checkForOutOfBoundAccess, and the global constant is inlined at the use site so the CUDA text carries the element value directly.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the last in-bounds index of a four-element array is accepted
// by the Phase B out-of-bound validation, and pairs that with the CUDA
// forcing of `inlineGlobalConstantsForLegalization`: because nvrtc
// rejects dynamic `__device__` initialization, the array constant is
// inlined at every use, so index 3 emits as the literal element value
// rather than a subscript on a global. The CHECK asserts the folded
// literal reaches the buffer store.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;
static const int arr[4] = { 1, 2, 3, 4 };

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = arr[3];
}

// CHECK: outBuf{{[A-Za-z0-9_]*}}){{.*}} = int(4);
