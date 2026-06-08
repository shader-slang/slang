//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#declaration-statement
//META: doc_section_digest=c02b3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2
//META: purpose=Variable declarations using let, var, and typed form are valid in statement context.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that let, var, and typed variable declarations are all valid statements.
// Each form declares a variable and its value is observable via printf.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let x = 10;
    var y = x + 1;
    int z = y - x;
    // CHECK: 10
    printf("%d\n", x);
    // CHECK: 11
    printf("%d\n", y);
    // CHECK: 1
    printf("%d\n", z);
}
