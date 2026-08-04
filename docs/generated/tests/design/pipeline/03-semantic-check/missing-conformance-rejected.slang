//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=22b10979ab5083a9c6d0c577210f37f6bb50a1e746c34c05d24d799e574a16a3
//META: purpose=A struct declared as conforming to an interface but missing a required member is rejected with a diagnostic naming the missing member.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says slang-check-conformance.cpp verifies a type
// satisfies the interfaces it claims. Verify the negative case:
// when the required member is not provided, the checker reports
// a missing-interface-member diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo { int bar(); }

struct S : IFoo {}
//CHECK:   ^^^^ missing interface member
//CHECK:   ^^^^ type 'S' does not provide required interface member 'bar'
// CHECK: E38100

void main() {}
