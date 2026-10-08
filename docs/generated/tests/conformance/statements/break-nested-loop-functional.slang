//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#break-statement
//META: doc_section_digest=e8ad1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0
//META: purpose=break in nested loops exits only the closest enclosing loop, not the outer loop.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that break exits only the innermost loop.
// Outer loop runs 3 times; inner loop breaks at j==1 each time, so inner count = 3.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int innerCount = 0;
    int outerCount = 0;
    for (int i = 0; i < 3; i++)
    {
        outerCount++;
        for (int j = 0; j < 10; j++)
        {
            if (j == 1)
                break;
            innerCount++;
        }
    }
    // Outer loop ran 3 times
    // CHECK: 3
    printf("%d\n", outerCount);
    // Inner loop ran once per outer iteration (j==0 only, then break at j==1)
    // CHECK: 3
    printf("%d\n", innerCount);
}
