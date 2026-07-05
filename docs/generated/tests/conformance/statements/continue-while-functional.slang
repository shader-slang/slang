//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#continue-statement
//META: doc_section_digest=f9be2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1
//META: purpose=continue in a while loop transfers to the condition check, not the start of the block.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that continue in a while-loop re-evaluates the condition.
// Counts n from 0 to 5, skipping the accumulator when n==2.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    int sum = 0;
    while (n < 5)
    {
        n++;
        if (n == 2)
            continue;
        sum += n;
    }
    // Sum = 1 + 3 + 4 + 5 = 13  (n==2 was skipped)
    // CHECK: 13
    printf("%d\n", sum);
}
