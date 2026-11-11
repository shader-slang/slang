//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=01a2d9637a96e0fd75d243d8f821bc31bba04bdf6901b24966dcfbc09af87ef9
//META: purpose=The checker rejects a `return` inside a deferred statement with E30110.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `return` may not appear inside a `defer` statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

int f(int x)
{
    defer { return 3; }
//CHECK:    ^^^^^^ 'return' inside defer
//CHECK:    ^^^^^^ 'return' must not appear inside a defer statement.
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = f(1);
}
