//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
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
