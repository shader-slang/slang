//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=A `let` declaration introduces an immutable variable whose value is observable but cannot be assigned to.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "`let` introduces an immutable variable".
// We verify that the declared value is readable with the expected value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let x = 7;
    let y : int = 42;
    // CHECK: 7
    // CHECK: 42
    printf("%d\n%d\n", x, y);
}
