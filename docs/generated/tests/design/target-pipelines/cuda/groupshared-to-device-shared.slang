//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=6125db2deed527f9850790324fd3a09c7d0cd76e0bbeee263f331f56a3527a81
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
