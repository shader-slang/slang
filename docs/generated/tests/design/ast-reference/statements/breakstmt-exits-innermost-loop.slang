//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=An unlabeled BreakStmt exits the innermost enclosing BreakableStmt; the outer loop continues its remaining iterations.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// BreakStmt's documented role is to jump out of the innermost
// BreakableStmt (loop or switch). Two nested loops with an inner
// break let us observe that the outer loop continues running after
// the inner break, which would not happen if break escaped further
// than the innermost loop.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int outerIters = 0;
    int innerHits = 0;
    for (int i = 0; i < 3; i = i + 1)
    {
        outerIters = outerIters + 1;
        for (int j = 0; j < 10; j = j + 1)
        {
            innerHits = innerHits + 1;
            if (j == 1) { break; }
        }
    }
    // CHECK: outerIters=3
    printf("outerIters=%d\n", outerIters);
    // each inner loop runs j=0,1 then breaks => 2 hits per outer => 6
    // CHECK: innerHits=6
    printf("innerHits=%d\n", innerHits);
}
