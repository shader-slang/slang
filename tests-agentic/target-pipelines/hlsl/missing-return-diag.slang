//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=9833b4c0a727318f2e9ddb19a0e7c49fb0434ebb154535e0c52405c196a4547c
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
