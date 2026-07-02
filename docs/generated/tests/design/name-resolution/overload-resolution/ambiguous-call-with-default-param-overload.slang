//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=Two overloads that both accept a single int argument (one directly, one via a defaulted trailing parameter) tie under the comparator and produce ambiguous-call (the doc's compareOverloadCandidateSpecificity step about default params does not break this tie in practice).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "compareOverloadCandidateSpecificity ... fewer
// default parameters > more" should break a tie between these
// two pick overloads, but the resolver actually reports the
// candidates as equally good and emits ambiguous-call. This is
// recorded as a doc gap in README.md.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int pick(int a)             { return 1; }
int pick(int a, int b = 99) { return 2; }

void main()
{
    int tag = pick(5);
//CHECK:          ^ ambiguous call to 'pick'
//CHECK: E39999
}
