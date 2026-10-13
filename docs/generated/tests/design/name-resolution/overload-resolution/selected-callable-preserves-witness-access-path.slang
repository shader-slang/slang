//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=d560d5c12f13cdbedf7330954dd7459f7f4df713fba134471a523c72319dde32
//META: purpose=_lookupMembersInSuperTypeFacets records a SuperType witness breadcrumb on each overloaded kind lookup item, and CompleteOverloadCandidate calls ConstructLookupResultExpr to replay the selected item because the candidate has no retainedExpr.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_lookupMembersInSuperTypeFacets` records the `SuperType` breadcrumb and subtype witness on each
// lookup item for the overloaded `kind` members on generic `T : IShape`. The resulting
// `OverloadedExpr` leaves each candidate without a `retainedExpr`. After selecting `kind(int)`,
// `CompleteOverloadCandidate` calls
// `ConstructLookupResultExpr`, which replays the breadcrumb while constructing the selected
// callable. The two conforming types return distinct tags, confirming that each call dispatched
// through its own witness table.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    int kind(int selector);
    int kind(float selector);
}

struct Square : IShape
{
    int kind(int selector) { return 4; }
    int kind(float selector) { return 40; }
}

struct Triangle : IShape
{
    int kind(int selector) { return 3; }
    int kind(float selector) { return 30; }
}

int describe<T : IShape>(T s) { return s.kind(0); }

void main()
{
    Square sq;
    Triangle tr;
    int a = describe(sq);
    int b = describe(tr);
    // CHECK: a=4 b=3
    printf("a=%d b=%d\n", a, b);
}
