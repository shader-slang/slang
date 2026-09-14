//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=96f5a93c2d3b7b0c3cfafeba9cc0e6087ddd11d34203934cd833e35b847de3f5
//META: purpose=A WhileStmt tests its predicate before each iteration; when the predicate is false at entry the body does not execute at all.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `while (cond)` tests its predicate before each iteration, so
// a predicate that is false on entry skips the body entirely. The CHECK
// asserts the counter reads 0; a tail-tested loop would have decremented
// it once.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int count = 0;
    while (count > 0)
    {
        count = count - 1;
    }
    // CHECK: count=0
    printf("count=%d\n", count);
}
