//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=4b4e0759c8a603e1b1c8bce1cbdccd4b05a30002e4de763fd5617a3d9f9b28cb
//META: purpose=CUDA emit opens with the slang-cuda-prelude.h include so the emitted text compiles under nvrtc.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The CUDASourceEmitter inherits from CPPSourceEmitter and emits a
// canonical prelude block that begins with a `#include` of
// `slang-cuda-prelude.h`. This is the marker we anchor the
// CUDA-emit prelude to.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include {{.*}}slang-cuda-prelude.h
