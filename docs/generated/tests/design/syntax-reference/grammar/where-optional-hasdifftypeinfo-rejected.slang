//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#generics-and-where-clauses
//META: doc_section_digest=41260c8f8bf7f0e8598932b33e0e91618368529d9e8fe3e7b7881fe0dd2a078f
//META: purpose=`optional` before a `__hasDiffTypeInfo(...)` where-term is diagnosed by the parser with E30315.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one where-term that does not tolerate a leading `optional`.
// CHECK asserts E30315, which only the parser's optional-prefix guard emits.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl

void f<T>(T t) where optional __hasDiffTypeInfo(T) {}

// CHECK: E30315
