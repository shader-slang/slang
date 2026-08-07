//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=5fafdc04391fe1499bf74bfe01420955a8364571f1b55e362cc671d3444538c9
//META: purpose=Verifies a scalar groupshared int becomes __device__ __shared__ on CUDA emit (the array-form smoke test already covers the aggregate case).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: groupshared of a scalar (no array). The
// `__device__ __shared__` lowering applies regardless of the
// type shape; the scalar form is the lower-edge case.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

groupshared int gs;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    if (ltid.x == 0U) gs = int(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = gs;
}

// CHECK: __device__ __shared__
// CHECK: __syncthreads
