//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
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
