//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=b22ae0db87266fb01a1be1ff4487019df27480bf09ff6476d416556c6388f8d4
//META: purpose=When a non-overloadable lookup resolves to multiple decls (two types of the same name reachable via different namespace imports), the checker reports ambiguous-reference.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The lookup doc says: "When a non-overloadable result resolves to
// multiple decls (e.g. two types with the same name reachable via
// different sibling scopes), the checker emits diagnostic
// `ambiguous-reference`". Verify the diagnostic fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

namespace A { struct X {}; }
//CHECK:             ^ candidate: struct A::X
namespace B { struct X {}; }
//CHECK:             ^ candidate: struct B::X

using namespace A;
using namespace B;

void main()
{
    X v;
/*CHECK:
    ^ ambiguous reference to 'X'
*/
}
