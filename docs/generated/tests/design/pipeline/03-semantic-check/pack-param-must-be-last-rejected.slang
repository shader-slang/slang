//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
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
