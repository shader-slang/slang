//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=The reversed `N == countof(Pack)` spelling is diagnosed with E30437 asking for `countof` on the left.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `3 == countof(T)` is recognized only to emit the targeted
// diagnostic; CHECK asserts E30437.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl

void f<each T>(expand each T t) where 3 == countof(T) {}

// CHECK: E30437
