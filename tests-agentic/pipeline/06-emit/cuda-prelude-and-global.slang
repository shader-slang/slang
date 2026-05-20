//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#cuda
//META: doc_section_digest=aec099a690ba0cb7b3cba6485d62487754f84f320420d4d5113995c23ee1a4bb
//META: purpose=The CUDA backend emits CUDA source that includes slang-cuda-prelude.h and exports the entry point as an extern "C" __global__ function.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The CUDA backend (slang-emit-cuda.{h,cpp}) emits a #include of the
// CUDA prelude (prelude/slang-cuda-prelude.h) and exports the
// compute entry point as `extern "C" __global__ void <main>_0(...)`.
// We match a substring of the prelude file name rather than the full
// path, which is absolute and machine-specific.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include {{.*}}slang-cuda-prelude.h
// CHECK: extern "C" __constant__ GlobalParams_{{[0-9]+}} SLANG_globalParams
// CHECK: extern "C" __global__ void main_{{[0-9]+}}()
