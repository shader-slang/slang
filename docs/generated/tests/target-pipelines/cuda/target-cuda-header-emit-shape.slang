//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
//META: purpose=-target cuda-header shares the CUDA IR pipeline with CUDASource and emits CUDA C++ text including the slang-cuda-prelude.h include and an extern "C" __global__ void kernel wrapper.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `CUDASource` and `CUDAHeader` are sibling CodeGenTarget values
// that share the entire IR pipeline inside `linkAndOptimizeIR`
// (intro paragraph). They differ only in artifact dispatch in
// Phase D — neither invokes nvrtc; both stop at CUDA C++ text.
// `-target cuda-header` is therefore expected to emit the same
// Phase-D markers (the prelude include and the extern "C"
// __global__ kernel wrapping) as `-target cuda`.

//TEST:SIMPLE(filecheck=CHECK):-target cuda-header -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: slang-cuda-prelude.h
// CHECK: extern "C" __global__ void main
