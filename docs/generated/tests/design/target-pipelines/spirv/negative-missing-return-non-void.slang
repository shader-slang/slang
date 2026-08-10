//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=daa747c870d798716cbdacfc1bb54e1466b0de4791e055a44205e8e6a56cbf91
//META: purpose=Phase B's checkForMissingReturns (table row 24, gated on reqSet.missingReturn) fires for a non-void function that does not return on every path — the error fires before SPIR-V emit.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: Phase B step 24 — checkForMissingReturns — gated on
// reqSet.missingReturn and shouldRunNonEssentialValidation. A
// non-void function with a missing-return path is rejected before
// SPIR-V emit.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

int compute(int x)
/*CHECK:
    ^^^^^^^ non-void function must return
*/
// CHECK: E41009
{
    if (x > 0)
        return x;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = compute(int(tid.x));
}
