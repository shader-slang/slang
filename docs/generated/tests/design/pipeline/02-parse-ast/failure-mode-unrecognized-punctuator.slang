//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
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
