//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=A CallableDecl's scope contains its parameter decls; the parameter is reachable from the function body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The scope-bearing table notes that CallableDecl's scope "contains
// the parameter decls". Verify by reading the parameter from inside
// the body.

//TEST:INTERPRET(filecheck=CHECK):

int square(int n)
{
    return n * n;
}

void main()
{
    int r = square(7);
    // CHECK: r=49
    printf("r=%d\n", r);
}
