//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=bae02bfd9f0c35f37e81a96d88e88829e442c28a56e0f30d0bb769febf165f85
//META: purpose=Negative: a non-void function missing a return triggers the `checkForMissingReturns` (Phase B, reqSet.missingReturn) diagnostic before HLSL emit.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Phase B includes `checkForMissingReturns` gated on
// `reqSet.missingReturn`. A function declared to return `int`
// with no return statement must fail before reaching HLSL emit.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int helper(int x)
/*CHECK:
    ^^^^^^ non-void function does not return in all cases
*/
// CHECK: W41010
{
    int y = x * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = helper(int(tid.x));
}
