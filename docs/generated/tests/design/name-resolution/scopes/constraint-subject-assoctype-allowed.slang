//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=5c32316c757a32af0aa6a61ee5f325f4830e8f73bc176d8904fdb4ddc1d8d758
//META: purpose=A `__constraint` whose subject is an associated type reached through `This` (e.g. `This.A : IBar`) is permitted, unlike a bare-This subject.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out next to the bare-This rejection: `This.A` as a
// constraint subject is accepted, and the constraint is usable — the
// generic body calls IBar's requirement on T.A, which resolves only if
// the This.A constraint registered as an interface requirement.

//TEST:INTERPRET(filecheck=CHECK):

interface IBar
{
    static int tag();
}

interface IHasA
{
    associatedtype A;
    __constraint This.A : IBar;
}

struct Bar21 : IBar
{
    static int tag() { return 21; }
}

struct HasA : IHasA
{
    typealias A = Bar21;
}

int readTag<T : IHasA>()
{
    return T.A.tag();
}

void main()
{
    int t = readTag<HasA>();
    // CHECK: t=21
    printf("t=%d\n", t);
}
