//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:55Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#preludes
//META: doc_section_digest=aa6dfad5c535c2a139def7eece16399b9abb61a5dab99ce3129b1ab73720d4de
//META: purpose=Emitted CUDA output references the CUDA prelude — a compute entry point is emitted with `extern "C"` linkage and `__global__` annotation defined by slang-cuda-prelude.h.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `prelude/slang-cuda-prelude.h` supplies the CUDA-side runtime
// helpers and macros. The `-target cuda` emit references the
// prelude and marks the entry point with `extern "C"` and
// `__global__`, which observably surface in the emitted text.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CUDA: extern "C"
// CUDA: __global__
