//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=003e69c99073f5634eddc84cdfb1b66685b97f4c42311978531b11906df8dbf4
//META: purpose=A `;` as the else branch of an `if` also yields an EmptyStmt and the potentially-unintended-empty-statement warning, because the parent statement is an IfStmt for either branch.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the warning fires for `else ;` and not for `else {}`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0) outputBuffer[0] = 1; else ;
//CHECK:                                      ^ potentially unintended empty statement
//CHECK:                                      ^ potentially unintended empty statement at this location; use {} instead.
    if (tid.x == 0) {} else {}
}
