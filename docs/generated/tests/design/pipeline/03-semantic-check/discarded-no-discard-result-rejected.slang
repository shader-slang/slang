//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=Discarding the result of a call to a `[NoDiscard]`-marked function is rejected with the discarded-no-discard-result diagnostic (E30059).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Failure modes section names maybeDiagnoseDiscardedNoDiscardResult, which
// fires when the result of a `[NoDiscard]` function is thrown away as a bare
// expression statement. Verify the diagnostic names the discarded call and
// cites code E30059. The caret lands on the discarded call; because the call
// starts before column 10, the caret is written in a block-comment form.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[NoDiscard]
int freeFunc() { return 1; }

[numthreads(1, 1, 1)]
void main()
{
    freeFunc();
/*CHECK:
            ^ result of '[NoDiscard]' function is discarded
            ^ the result of calling 'freeFunc' is discarded; this function is marked '[NoDiscard]'.
*/
// CHECK: E30059
}
