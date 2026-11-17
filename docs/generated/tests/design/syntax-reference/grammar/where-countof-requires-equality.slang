//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=A comparison other than `==` after `countof(Pack)` is consumed and reported as E30436.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `countof(T) < 3` is diagnosed as requiring `==`; CHECK asserts E30436.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl

void f<each T>(expand each T t) where countof(T) < 3 {}

// CHECK: E30436
