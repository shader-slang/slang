//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=96f5a93c2d3b7b0c3cfafeba9cc0e6087ddd11d34203934cd833e35b847de3f5
//META: purpose=A ForStmt whose predicate is true exactly once executes the body exactly once and then exits; both initialStatement and sideEffectExpression run before the predicate fails on the second test.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: 1-iteration edge. The init runs once (i=5), the body
// runs once (hits=1), and then sideEffectExpression bumps i to 6,
// after which the predicate i<6 is false. A mis-lowering that
// dropped sideEffectExpression would run forever; one that swapped
// pre/post-test would run zero or two times.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int hits = 0;
    for (int i = 5; i < 6; i = i + 1)
    {
        hits = hits + 1;
    }
    // CHECK: hits=1
    printf("hits=%d\n", hits);
}
