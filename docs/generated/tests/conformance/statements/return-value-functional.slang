//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#return-statement
//META: doc_section_digest=a0cf3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2
//META: purpose=return expr supplies the return value to the caller; the returned value is visible at the call site.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `return expr` supplies the function result; the caller observes the returned value.
//TEST:INTERPRET(filecheck=CHECK):

int square(int x) { return x * x; }

void main()
{
    int r = square(7);
    // CHECK: 49
    printf("%d\n", r);
}
