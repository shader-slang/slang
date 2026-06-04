//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#block-statement
//META: doc_section_digest=d13c4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3
//META: purpose=A block statement provides local scoping; declarations in the block are not visible outside.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a block statement groups statements and scopes declarations locally.
// Inner `x` shadows the outer and is not visible outside the block.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 1;
    {
        int x = 99;
        // CHECK: 99
        printf("%d\n", x);
    }
    // After block, outer x is visible again
    // CHECK: 1
    printf("%d\n", x);
}
