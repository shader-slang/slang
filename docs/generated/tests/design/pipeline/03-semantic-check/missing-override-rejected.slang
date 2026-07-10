//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
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
