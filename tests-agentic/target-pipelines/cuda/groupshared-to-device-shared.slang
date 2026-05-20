//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
//META: purpose=A groupshared array becomes a __device__ __shared__ global in CUDA emit and GroupMemoryBarrierWithGroupSync lowers to __syncthreads.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDA's CPPSourceEmitter/CUDASourceEmitter writes `groupshared` as
// `__device__ __shared__` and the HLSL barrier intrinsic becomes
// the CUDA `__syncthreads()` builtin.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

groupshared float gs[64];
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    gs[ltid.x] = float(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = gs[ltid.x];
}

// CHECK: __device__ __shared__
// CHECK: __syncthreads
