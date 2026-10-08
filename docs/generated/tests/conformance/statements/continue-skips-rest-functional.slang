//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#continue-statement
//META: doc_section_digest=f9be2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1
//META: purpose=continue transfers control to the start of the next loop iteration, skipping remaining body.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that continue skips the rest of the loop body and starts the next iteration.
// Sum only accumulates for odd i (0..4); even i are skipped via continue.
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
    // Odd values: 1 + 3 = 4
    // CHECK: 4
    printf("%d\n", sum);
}
