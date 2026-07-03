//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ee0e6ce7e7ab498c5056d2e357cc8f20c0e28b6f8867c47be3c3588bab6092bf
//META: purpose=Phase B's checkForRecursiveFunctions (table row 22) rejects a self-recursive call with E55201 before Phase C/D legalization can run.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: Phase B step 22 — checkForRecursiveFunctions — fires
// before SPIR-V emit. The diagnostic comes from the IR pipeline,
// not the SPIR-V backend.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

int factorial(int n)
{
    return n <= 1 ? 1 : n * factorial(n - 1);
//CHECK:                             ^ recursion not allowed
// CHECK: E55201
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = factorial(int(tid.x));
}
