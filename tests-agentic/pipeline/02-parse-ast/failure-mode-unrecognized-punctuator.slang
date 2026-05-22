//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=679b7b8ec60987b15c9c416bbbe2090c15a395b4d26dc35cb8d230bdc6e1f986
//META: purpose=Token-level errors not previously reported by the lexer (here, an `@` punctuator inside a struct declaration) surface at parse time as parse errors via the DiagnosticSink.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `@` inside the struct body is a token the lexer accepts but the
// parser cannot place. The doc's `## Failure modes` section names
// this as a parse-time diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S { int a; @ int b; };
//CHECK: unexpected '@'
//CHECK: E20003
