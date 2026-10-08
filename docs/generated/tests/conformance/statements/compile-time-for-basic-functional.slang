//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#compile-time-for-statement
//META: doc_section_digest=b1d04c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3
//META: purpose=$for(i in Range(0, 4)) body instantiates the body with i bound to 0, 1, 2, 3.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that $for unrolls the body once per value in Range(initial, upper).
// Sum of 0+1+2+3 = 6 confirms all 4 instantiations ran.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    $for(i in Range(0, 4))
    {
        sum += i;
    }
    // CHECK: 6
    printf("%d\n", sum);
}
