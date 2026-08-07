//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=cec0af169cdea7704c970ac20490303750b9aad97be14f4aa1d9723019cca840
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
