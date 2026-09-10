//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#cuda
//META: doc_section_digest=8f41f8dafebbe6baa6266ab98725f783f2c94164b259fe23be1fe98a79c0cdc4
//META: purpose=The CUDA backend emits CUDA source that includes `slang-cuda-prelude.h` and exposes the compute entry point as an `extern "C" __global__` kernel.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CUDA backend produces text nvrtc would accept. CHECK asserts
// the prelude include (matched by filename, since the emitted path is
// absolute), the CUDA launch qualifier on the entry point, the `__device__`
// qualifier the backend must add to an ordinary callee, and the
// `__constant__`-backed global-parameter block CUDA uses in place of a
// binding table.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int addOne(int v)
{
    return v + 1;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = addOne(int(tid.x));
}

// CUDA: slang-cuda-prelude.h
// CUDA-DAG: __device__ int addOne_{{[0-9]+}}(
// CUDA-DAG: extern "C" __constant__ GlobalParams_{{[0-9]+}} SLANG_globalParams;
// CUDA-DAG: extern "C" __global__ void main_{{[0-9]+}}()
