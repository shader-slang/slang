//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=57d9f151be60063d08c61244f7768487d8eaebc83717628353b6150f280dd55a
//META: purpose=A call whose argument list cannot be matched against a generic's parameter list is reported as a specialization arity mismatch (E30438) naming both counts.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GenericArityMismatch arm. Argument-to-parameter matching fails
// before any inference can run, so the failure is captured with the two counts
// and formatted only once this candidate is selected. CHECK pins E30438 and
// the companion "see declaration" note.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Pack<each T>
{
}

void f<each T>(expand each T args, Pack<expand each T> p)
{
}

void main()
{
    Pack<int> p;
    f(1, 2.0f, p);
//CHECK: E30438
//CHECK: see declaration of
}
