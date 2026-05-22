//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=Decl checking rejects a generic parameter declared after a variadic pack parameter (E30522).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Variadic-pack boundary: an `each T` pack parameter must be the
// last in the generic parameter list.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f<each T, U>(U u) {}
//CHECK:       ^ generic parameter after a variadic pack parameter is not allowed
//CHECK:       ^ generic parameter 'U' cannot appear after a variadic pack parameter
// CHECK: E30522

void main() {}
