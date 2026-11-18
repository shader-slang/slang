//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=01a2d9637a96e0fd75d243d8f821bc31bba04bdf6901b24966dcfbc09af87ef9
//META: purpose=A `break` or `continue` that would leave a deferred statement is rejected by the checker (E30108 / E30109).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `break` and `continue` may not exit a `defer` statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    for (int i = 0; i < 2; i++)
    {
        defer { break; }
//CHECK:        ^^^^^ 'break' inside defer
//CHECK:        ^^^^^ 'break' must not appear inside a defer statement.
    }
    for (int j = 0; j < 2; j++)
    {
        defer { continue; }
//CHECK:        ^^^^^^^^ 'continue' inside defer
//CHECK:        ^^^^^^^^ 'continue' must not appear inside a defer statement.
    }
    outputBuffer[0] = 1;
}
