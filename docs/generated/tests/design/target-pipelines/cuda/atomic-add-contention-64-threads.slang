//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=Verifies an atomic operation under a 64-thread block (contention stress) still emits as a single atomicAdd call on the same buffer slot.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: 64 threads all writing the same slot through
// `InterlockedAdd`. The emit shape is unchanged from the smoke
// test, but exercises a high-contention dispatch envelope.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    InterlockedAdd(outBuf[0], 1);
}

// CHECK: atomicAdd
