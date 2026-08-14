//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=9f957015017e5369dca04b3ddeac3c5f9b0cfdeca3888bbec30602504004bdac
//META: purpose=A non-conformance generic constraint such as `where T == int` that cannot be discharged is reported as an unsatisfied constraint (E30440) rendering the constraint text.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GenericConstraintNotSatisfied arm, the fallback used for every
// constraint kind other than interface conformance. `T` infers to `float` from
// the argument, which cannot satisfy the equality constraint; the stored
// constraint declaration is rendered into the message. CHECK pins E30440, the
// constraint-declaration note this arm adds, and the signature note.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<T>(T a) where T == int
{
}

void main()
{
    f(1.5f);
//CHECK: E30440
//CHECK: see generic constraint declaration
//CHECK: see declaration of
}
