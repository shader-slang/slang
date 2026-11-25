//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=19ad073ab979d4a331ac1f5fc84f58af039847fad8e90dbe9db085b55e74e2b0
//META: purpose=The dedicated `__constraint` keyword inserts a GenericTypeConstraintDecl directly into the interface scope, equivalent to the two associatedtype clause forms, so generic code can call IBar's requirement on T.A.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third constraint surface form: a standalone `__constraint`
// on the associated type is a parallel requirement member of the
// interface scope. The generic call through T.A.tag() succeeds only if
// the keyword-form constraint registered like the clause forms do.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar
{
    static int tag();
}

interface IHasA
{
    associatedtype A;
    __constraint A : IBar;
}

struct Bar9 : IBar
{
    static int tag() { return 9; }
}

struct HasA : IHasA
{
    typealias A = Bar9;
}

int readTag<T : IHasA>()
{
    return T.A.tag();
}

void main()
{
    int t = readTag<HasA>();
    // CHECK: t=9
    printf("t=%d\n", t);
}
