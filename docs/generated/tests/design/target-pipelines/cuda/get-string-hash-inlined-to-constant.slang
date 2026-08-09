//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=performTypeInlining and checkGetStringHashInsts both run for CUDA because a CUDA artifact is not CPU-like, so getStringHash on a string literal reduces to a plain integer constant in the emitted kernel.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the observable consequence of CUDA taking the not-CPU-like
// branch in Phase B. `performTypeInlining` inlines string calls and
// returns so every `getStringHash` operand ends up a string literal, and
// `checkGetStringHashInsts` then validates that; a CUDA compile that
// skipped those would leave a string-typed value the emitter cannot
// write. The CHECK asserts the buffer store receives a bare integer
// literal — the hash was folded at compile time.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = getStringHash("slang");
}

// CHECK: outBuf{{[A-Za-z0-9_]*}}){{.*}} = {{[0-9]+}};
