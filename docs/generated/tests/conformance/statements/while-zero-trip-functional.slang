//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#while-statement
//META: doc_section_digest=c68b9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8
//META: purpose=while(false) body executes the body zero times.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the zero-trip while loop: condition is false from the start.
// Counter stays 0, confirming the body is never entered.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int count = 0;
    while (false)
        count++;
    // CHECK: 0
    printf("%d\n", count);
}
