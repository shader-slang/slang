//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=677c751462cdb5ae5189c32a0f57ea5105199a379d69be08450135057a10b2db
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
