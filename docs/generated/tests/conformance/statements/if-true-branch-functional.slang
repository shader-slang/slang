//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#if-statement
//META: doc_section_digest=f35e6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5
//META: purpose=if(cond) stmt executes the then-branch when cond is true, skips it when false.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `if(cond) stmt` executes the body when the condition is true.
// Also verifies that when the condition is false, the body is not executed.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 0;
    if (true)
        x = 1;
    // CHECK: 1
    printf("%d\n", x);

    int y = 0;
    if (false)
        y = 1;
    // CHECK: 0
    printf("%d\n", y);
}
