//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
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
}
