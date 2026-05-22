//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ee0e6ce7e7ab498c5056d2e357cc8f20c0e28b6f8867c47be3c3588bab6092bf
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
