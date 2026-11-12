//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
//META: purpose=A per-id severity override may not lower a diagnostic that is already at `Error`: `-warnings-disable` on an error id is rejected with E31111.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the override step refuses to demote a diagnostic whose declared
// severity is already Error. 30015 is undefined-identifier, an error entry;
// requesting `-warnings-disable 30015` is rejected rather than silently applied.
// CHECK asserts the rejection is reported as E31111 and CHECK-NOT asserts the
// error was not, in fact, silenced into nothing by a partially-applied override.

//TEST:SIMPLE(filecheck=CHECK,exit-code=1):-target hlsl -entry main -stage compute -warnings-disable 30015

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = notDeclaredAnywhere;
}

// CHECK: E31111
// CHECK: unknown diagnostic
// CHECK-NOT: E41024
