//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=6125db2deed527f9850790324fd3a09c7d0cd76e0bbeee263f331f56a3527a81
//META: purpose=A compute entry point is emitted as an extern "C" __global__ void kernel because CUDA is a valid CUDA kernel.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CUDA's Phase A removes the `moveEntryPointUniformParamsToGlobalScope`
// and `removeTorchAndCUDAEntryPoints` passes (CUDA entry points are
// already valid CUDA kernels). The emitter wraps the entry point in
// `extern "C" __global__ void <name>()`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: extern "C" __global__ void main
