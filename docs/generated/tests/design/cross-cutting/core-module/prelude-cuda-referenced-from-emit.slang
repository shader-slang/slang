//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#preludes
//META: doc_section_digest=a7e8e82107afcf2792c8394b7f5dff917197cf8708f197b2045955821e3cb62b
//META: purpose=The CUDA prelude is referenced from -target cuda output: the emitted text pulls in slang-cuda-prelude.h and declares the entry point with extern "C" __global__.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CUDA back end reaches its prelude the same output-side
// way the C++ back end does. The shader is prelude-agnostic; the
// CHECKs pin the slang-cuda-prelude.h reference plus the
// extern "C" __global__ entry-point form whose helper types come from
// that header, which together show the emitted text depends on the
// prelude rather than on anything the front end checked.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CUDA: slang-cuda-prelude.h
// CUDA: extern "C" __global__ void computeMain
