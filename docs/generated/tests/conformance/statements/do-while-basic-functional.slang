//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#do-while-statement
//META: doc_section_digest=d79c0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9
//META: purpose=do body while(cond) executes the body at least once before checking the condition.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that do-while executes the body before testing the condition.
// Even with an initially-false condition, the body executes once.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // Body always executes at least once
    int count = 0;
    do {
        count++;
    } while (false);
    // CHECK: 1
    printf("%d\n", count);

    // Normal multi-trip do-while
    int n = 0;
    do {
        n++;
    } while (n < 3);
    // CHECK: 3
    printf("%d\n", n);
}
