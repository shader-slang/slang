//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=ea81eee9c248c045a204cf03769d29438ea6b24a6cbd441a4c42b3efc37f9f53
//META: purpose=The `associatedtype A where A : IBar` where-clause form places its GenericTypeConstraintDecl in the enclosing interface's scope like the inheritance-clause form, so generic code can call IBar's requirement on T.A.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second constraint surface form: the where-clause on the
// associatedtype produces the same interface-scope requirement member as
// the inheritance-clause spelling. The generic call through T.A.tag()
// succeeds only if the where-clause constraint registered.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar
{
    static int tag();
}

interface IHasA
{
    associatedtype A where A : IBar;
}

struct Bar8 : IBar
{
    static int tag() { return 8; }
}

struct HasA : IHasA
{
    typealias A = Bar8;
}

int readTag<T : IHasA>()
{
    return T.A.tag();
}

void main()
{
    int t = readTag<HasA>();
    // CHECK: t=8
    printf("t=%d\n", t);
}
