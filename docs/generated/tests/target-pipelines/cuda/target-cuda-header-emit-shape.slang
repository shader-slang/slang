//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
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
