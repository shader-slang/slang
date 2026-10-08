//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#for-statement
//META: doc_section_digest=b57a8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7
//META: purpose=In a for-loop with a side-effect expression, the increment runs even when continue is used.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the for-loop increment expression executes after a continue statement.
// Doc: "the side effect expression is evaluated when continue is used".
// Loop runs i=0..4, skipping the accumulator on even i; increment still fires each time.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 0; i < 5; i++)
    {
        if (i % 2 == 0)
            continue;
        sum += i;
    }
    // odd values 1 + 3 = 4, and i reaches 5 confirming increment ran on continue
    // CHECK: 4
    printf("%d\n", sum);
}
