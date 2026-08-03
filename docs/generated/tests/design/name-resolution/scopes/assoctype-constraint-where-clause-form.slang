//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=677c751462cdb5ae5189c32a0f57ea5105199a379d69be08450135057a10b2db
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
