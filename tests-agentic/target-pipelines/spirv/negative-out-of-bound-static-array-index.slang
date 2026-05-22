//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=Phase B's checkForOutOfBoundAccess fires before SPIR-V emit when a static array is indexed past its declared bound; the documented diagnostic is reported on the IR pipeline (not the SPIR-V backend).
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: an out-of-bounds compile-time constant index on a
// fixed-size local array is rejected by checkForOutOfBoundAccess
// (Phase B, table row 23). The error fires before SPIR-V emit.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[3];
    arr[0] = 0;
    arr[1] = 1;
    arr[2] = 2;
    outBuf[tid.x] = arr[5];
//CHECK:               ^ array index out of bounds
// CHECK: E30029
}
