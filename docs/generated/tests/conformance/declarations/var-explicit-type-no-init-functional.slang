//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A `var` with an explicit type but no initializer is allowed; the variable is uninitialized until explicitly assigned.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `var x : int` (explicit type, no initializer) compiles and is writable.
// After declaring without an initializer, `x` is assigned 55; 55 confirms the
// variable was live and held the assigned value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var x : int;
    x = 55;
    // CHECK: 55
    printf("%d\n", x);
}
