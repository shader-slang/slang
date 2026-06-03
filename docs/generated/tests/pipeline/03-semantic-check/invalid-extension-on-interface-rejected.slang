//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=Inheritance checking rejects an `extension` directly on an interface type (E30852).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Extension target boundary: `extension I { ... }` directly on an
// interface is rejected with E30852 (use a generic extension
// `extension<T:I> T {...}` instead).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface I { int m(); }
extension I { int m() { return 0; } }
//CHECK:  ^ cannot extend interface type 'I'
//CHECK:  ^ cannot extend interface type 'I'. consider using a generic extension: `extension<T:I> T {...}`
// CHECK: E30852

void main() {}
