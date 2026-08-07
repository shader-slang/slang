//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=1d8e7a1c30049e4d9b67c4beb3ca230fe889d9d80802b14075e9c38d36c53d4c
//META: purpose=A static array index past the upper edge of a fixed-size array is rejected by Phase B with E30029
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Above-the-upper-edge of a fixed-size array index: Phase B's
// checkForOutOfBoundAccess catches the constant index and emits
// E30029 before WGSL emit. The diagnostic is not attached to a
// specific source line, so we match by message text only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = { 1, 2, 3, 4 };
    outBuf[tid.x] = arr[10];
}

//CHECK: array index out of bounds
// CHECK: E30029
