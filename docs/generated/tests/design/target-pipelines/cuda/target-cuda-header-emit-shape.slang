//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=5fafdc04391fe1499bf74bfe01420955a8364571f1b55e362cc671d3444538c9
//META: purpose=-target cuh (CUDA header) shares the CUDA IR pipeline with CUDASource and emits a C++ header containing the slang-cuda-prelude.h include and `extern "C"` global-parameter declarations; the kernel body is emitted by the sibling `-target cuda` target.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `CUDASource` and `CUDAHeader` are sibling CodeGenTarget values
// that share the entire IR pipeline inside `linkAndOptimizeIR`
// (intro paragraph). They differ only in artifact dispatch in
// Phase D — neither invokes nvrtc; both stop at CUDA C++ text.
// The CLI spelling for CUDAHeader is `-target cuh`. It emits a
// header-only artifact: the prelude `#include` plus `extern "C"`
// declarations for the per-entry-point global-parameter block;
// the kernel body itself is emitted by the sibling `-target cuda`.

//TEST:SIMPLE(filecheck=CHECK):-target cuh -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK-DAG: slang-cuda-prelude.h
// CHECK-DAG: extern "C" __constant__
