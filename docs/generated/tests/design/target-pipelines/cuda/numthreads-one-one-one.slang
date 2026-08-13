//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=5fafdc04391fe1499bf74bfe01420955a8364571f1b55e362cc671d3444538c9
//META: purpose=Verifies the minimum-thread dispatch shape [numthreads(1,1,1)] still produces an extern "C" __global__ void main_0 entry point on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: minimum [numthreads] dispatch shape. CUDA kernel
// wrapping is independent of the thread-group size; the single-
// thread shape still emits a valid kernel signature.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: extern "C" __global__ void main
