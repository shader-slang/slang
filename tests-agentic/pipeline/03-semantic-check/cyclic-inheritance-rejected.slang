//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Inheritance checking rejects a struct that names itself as its base ("cyclic reference in inheritance graph").
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Inheritance-checking boundary: self-inheritance is a degenerate
// cycle and is reported via the cyclic-reference-in-inheritance
// diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct A : A {}
/*CHECK:
       ^ cyclic reference in inheritance graph 'A'
*/
//CHECK: E39999

void main() {}
