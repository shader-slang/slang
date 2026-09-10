//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
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
