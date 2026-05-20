//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
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

void main() {}
