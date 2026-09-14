//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=96f5a93c2d3b7b0c3cfafeba9cc0e6087ddd11d34203934cd833e35b847de3f5
//META: purpose=A DoWhileStmt tests its predicate after each iteration; the body executes at least once even when the predicate is false at entry.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `do ... while (cond);` tests its predicate after the body,
// so the body runs at least once even when the predicate is false from
// the start. The CHECK asserts the counter reads 1; a head-tested loop
// would leave it at 0.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    do
    {
        n = n + 1;
    } while (n < 0);
    // CHECK: n=1
    printf("n=%d\n", n);
}
