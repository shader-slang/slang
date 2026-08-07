//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#error-recovery
//META: doc_section_digest=490a9dc1fa74d15b5c32d617920911d5d9b67dbf1c0dd0c9637afde42b648b85
//META: purpose=On an unexpected token the parser emits a diagnostic, then uses skip-to-synchronization-point heuristics to resume, so a second later error in the same file is also reported.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Both function bodies contain the unexpected punctuator `@@`. Each
// produces a parse diagnostic at the offending token, and the parser
// recovers to a synchronization point (here, the closing `}` of the
// containing body) so it can proceed to the next decl and report the
// second error too.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void foo() { return @@; }
//CHECK: syntax error

void bar() { return @@; }
//CHECK: syntax error
//CHECK: E20002
