//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
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
