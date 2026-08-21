//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=6125db2deed527f9850790324fd3a09c7d0cd76e0bbeee263f331f56a3527a81
//META: purpose=Non-entry-point Slang functions are emitted with __device__ decoration in CUDA emit so nvrtc compiles them as device code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The CUDASourceEmitter writes `__device__` (and `__global__` for
// entry points) decorations so nvrtc routes non-kernel functions
// to the device side of the CUDA C++ compilation.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

int square(int x) { return x * x; }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = square(int(tid.x));
}

// CHECK: __device__ {{.*}}square
