//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=9f957015017e5369dca04b3ddeac3c5f9b0cfdeca3888bbec30602504004bdac
//META: purpose=Constraint checking rejects a non-interface type used as a generic constraint (E30401).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the constraint solver only admits interfaces as bounds. A
// struct has no requirement set to witness, so naming it after `:` in a
// generic parameter list cannot produce a solvable constraint. CHECK pins
// E30401 on the offending bound.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct NotAnInterface
{
    int x;
}

int f<T : NotAnInterface>(T v)
{
    return 1;
}
//CHECK: E30401

void main()
{
}
