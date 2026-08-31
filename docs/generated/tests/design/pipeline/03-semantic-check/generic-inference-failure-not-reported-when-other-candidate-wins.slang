//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=9f957015017e5369dca04b3ddeac3c5f9b0cfdeca3888bbec30602504004bdac
//META: purpose=A generic candidate whose specialization failed produces no diagnostic when overload resolution selects a different candidate, because the failure is only formatted for the selected one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "captured eagerly, reported lazily" half of the claim. The
// generic `pick<T : IFoo>` cannot be specialized for `S`, but a non-generic
// `pick(S)` overload is applicable and wins. If the recorded failure were
// reported at capture time the program would not compile at all; CHECK
// asserts the non-generic overload ran and the program produced a value.

//TEST:INTERPRET(filecheck=CHECK):

interface IFoo
{
    int get();
}

struct S
{
    int x;
}

int pick<T : IFoo>(T a)
{
    return a.get();
}

int pick(S a)
{
    return 8;
}

void main()
{
    S s;
    // CHECK: v=8
    printf("v=%d\n", pick(s));
}
