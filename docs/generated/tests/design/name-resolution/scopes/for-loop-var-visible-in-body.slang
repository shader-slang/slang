//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
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
