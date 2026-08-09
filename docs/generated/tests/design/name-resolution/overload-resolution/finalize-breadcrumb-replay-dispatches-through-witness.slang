//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=a916857bbe07c220fc4b183b2a1794d39a799be9e41fa992dc6a49d37be94635
//META: purpose=CompleteOverloadCandidate rebuilds the callee by replaying the lookup item's breadcrumb chain, so an interface-bounded generic call dispatches to the conforming type's implementation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the callee-construction half of the finalize phase for an
// interface-requirement candidate: the breadcrumb chain the lookup left behind
// is what turns `s.kind()` into a witness-table dispatch on T. Two conforming
// types return distinct tags; CHECK asserts each call reached its own
// implementation.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape { int kind(); }

struct Square : IShape { int kind() { return 4; } }
struct Triangle : IShape { int kind() { return 3; } }

int describe<T : IShape>(T s) { return s.kind(); }

void main()
{
    Square sq;
    Triangle tr;
    int a = describe(sq);
    int b = describe(tr);
    // CHECK: a=4 b=3
    printf("a=%d b=%d\n", a, b);
}
