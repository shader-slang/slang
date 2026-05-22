//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=Conformance checking rejects a member that overrides an interface default without the `override` keyword (E30853).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When an interface provides a default implementation, a conforming
// struct that supplies its own must mark the method `override`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface I { int m() { return 0; } }
struct S : I { int m() { return 1; } }
//CHECK:           ^ missing 'override' keyword for methods that overrides the default implementation in the interface
// CHECK: E30853

void main() {}
