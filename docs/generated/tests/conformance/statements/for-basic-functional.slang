//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#for-statement
//META: doc_section_digest=b57a8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7
//META: purpose=for(init;cond;incr) body iterates exactly the right number of times.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies basic for-loop: sums 0..4 = 10, confirming 5 iterations.
// Variable declared in init (`i`) is visible throughout the for-statement.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 0; i < 5; i++)
        sum = sum + i;
    // CHECK: 10
    printf("%d\n", sum);
}
