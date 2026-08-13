//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
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
