//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
//META: purpose=Constraint checking rejects a generic value parameter whose type is not an integer or enum (E30624).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Generic value parameters must be int or enum typed; a `float` is
// rejected with E30624.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct V<let x : float> { int y; }
//CHECK:     ^ generic value parameter type 'float' is not supported; only integer and enum types are allowed
// CHECK: E30624

void main() {}
