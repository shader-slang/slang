//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=6231dfdd4bb6c753a6bf52eced2d80ecaefb0372d0cea815a341d7f7d1af11d6
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
