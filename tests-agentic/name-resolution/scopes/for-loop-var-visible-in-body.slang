//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=The ForStmt's scope holds the init variable; the body sees it for the duration of the loop.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Positive companion: from inside the ForStmt's body the loop
// variable `i` is reachable via the body's parent scope.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 0; i < 4; ++i)
    {
        sum = sum + i;
    }
    // CHECK: sum=6
    printf("sum=%d\n", sum);
}
