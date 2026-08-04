//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=cc891d0f95a64fa8f584e7573e2e02ccc1d048aeae30b2a876b01210f6bd45a3
//META: purpose=Token-level errors not previously reported by the lexer (here, an `@` punctuator inside a struct declaration) surface at parse time as parse errors via the DiagnosticSink.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `@` inside the struct body is a token the lexer accepts but the
// parser cannot place. The doc's `## Failure modes` section names
// this as a parse-time diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int a; @ int b; };
//CHECK:          ^ unexpected token
//CHECK:          ^ unexpected '@', expected identifier
//CHECK:                ^ unexpected token
//CHECK:                ^ unexpected identifier, expected ';'
//CHECK:                ^ declaration does not declare anything
