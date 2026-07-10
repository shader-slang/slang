//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=53fa67e8e5b5b7cc0d09c737e54d406ba6f9a31c67e2efff85428bbfda70a405
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
