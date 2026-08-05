//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=96f5a93c2d3b7b0c3cfafeba9cc0e6087ddd11d34203934cd833e35b847de3f5
//META: purpose=A ForStmt whose predicateExpression is false at entry executes zero iterations; the loop body does not run at all.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a `for` whose predicate is already false when first tested.
// Verifies the predicate is evaluated before the first iteration, so
// the body never runs. The CHECK asserts the counter stayed at 0.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int hits = 0;
    for (int i = 0; i < 0; i = i + 1)
    {
        hits = hits + 1;
    }
    // CHECK: hits=0
    printf("hits=%d\n", hits);
}
