//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#error-recovery
//META: doc_section_digest=ef1b32ca69300d934ea71780ab86e416b00a5b7222ec62aad6b9345794ab0303
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
