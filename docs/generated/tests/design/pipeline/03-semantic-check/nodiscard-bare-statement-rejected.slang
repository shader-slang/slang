//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=1690bc07bc351f6d57ef0cbfbdf90dd7a3a711d8e434d872c178e166712d1f85
//META: purpose=Discarding the result of a `[NoDiscard]` call written as a bare expression statement is rejected with E30059.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the base case of the discarded-result check: the call is the whole
// statement, so its value has nowhere to go. CHECK pins E30059, whose message
// names the callee and the attribute.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[NoDiscard]
int f()
{
    return 1;
}

void main()
{
    f();
//CHECK: E30059
}
