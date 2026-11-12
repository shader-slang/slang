//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=Using a member provided by an `optional` constraint without an `is` guard is diagnosed with E30403.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an optional conformance constraint must be tested with `is` before
// use: calling `t.g()` unguarded is rejected, CHECK asserts E30403.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl

interface IFoo { int g(); }

int f<T>(T t) where optional T : IFoo
{
    return t.g();
}

// CHECK: E30403
