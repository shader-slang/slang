//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=ea81eee9c248c045a204cf03769d29438ea6b24a6cbd441a4c42b3efc37f9f53
//META: purpose=The `associatedtype A : IBar` inheritance-clause form places its GenericTypeConstraintDecl in the enclosing interface's scope as a requirement member, so generic code constrained to the interface can call IBar's requirement on T.A.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the first of the three constraint surface forms: the clause
// on `associatedtype A : IBar` lands in the interface scope as a
// requirement, so `readTag<T : IHasA>` may call the IBar requirement on
// T.A. CHECK asserts the conforming type's tag value comes back, which
// requires the constraint to have registered.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar
{
    static int tag();
}

interface IHasA
{
    associatedtype A : IBar;
}

struct Bar7 : IBar
{
    static int tag() { return 7; }
}

struct HasA : IHasA
{
    typealias A = Bar7;
}

int readTag<T : IHasA>()
{
    return T.A.tag();
}

void main()
{
    int t = readTag<HasA>();
    // CHECK: t=7
    printf("t=%d\n", t);
}
