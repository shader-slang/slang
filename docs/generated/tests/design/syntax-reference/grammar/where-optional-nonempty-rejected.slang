//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=`optional nonempty(Pack)` parses but the checker rejects it with E30415.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `optional` is consumed on `nonempty(...)` but then rejected as
// meaningless by the checker; CHECK asserts E30415.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl

void f<each T>(expand each T t) where optional nonempty(T) {}

// CHECK: E30415
