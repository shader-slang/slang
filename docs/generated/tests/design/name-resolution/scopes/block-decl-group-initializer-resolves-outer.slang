//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=6f56ac76aaddef7a4076217c6b9406ddfe1d14ddd4d122a991eba3c769b781e4
//META: purpose=In a declaration group, each declarator stays hidden from lookup until the checker walks past it, so an earlier declarator's initializer resolves the outer same-named local.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The checker sets hiddenFromLookup on each member of a DeclGroup, not only
// on single declarations. In `int y = x, x = 20;` the initializer of `y`
// comes before the inner `x` is declared, so it resolves to the outer `x`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 10;
    {
        int y = x, x = 20;
        // CHECK: y=10 x=20
        printf("y=%d x=%d\n", y, x);
    }
}
