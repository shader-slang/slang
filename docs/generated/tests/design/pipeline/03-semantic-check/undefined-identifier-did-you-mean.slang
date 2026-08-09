//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=4c9f71a256907b7a79759ca00a313ad651ac24d85015af4f3fe1673fd66d9dde
//META: purpose=An undefined identifier that is one edit away from an in-scope name carries a conservative did-you-mean suggestion on the diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the similar-name suggestion attached to the undefined-identifier
// diagnostic. The misspelling drops a single trailing character from a long
// in-scope local, which is inside the length-scaled edit budget, so the
// suggestion must appear. CHECK pins E30015 and the suggestion note naming the
// intended identifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int myLongVariableName = 3;
    int y = myLongVariableNam;
//CHECK: E30015
//CHECK: did you mean 'myLongVariableName'?
}
