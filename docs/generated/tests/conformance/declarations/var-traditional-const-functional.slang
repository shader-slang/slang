//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#variables
//META: doc_section_digest=98b79b7c34fc8faf47b9b15881a4299669d58061046761d5503ddf01389e9d47
//META: purpose=Traditional `const` variable declarations are immutable; traditional mutable declarations (without `const`) are mutable.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Traditional variable declarations are immutable if they are
// declared with the `const` modifier, and are otherwise mutable."

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    const int MAX = 100;
    float y = 9.0;
    y = 3.14;
    // CHECK: 100
    // CHECK: 3.14
    printf("%d\n%g\n", MAX, y);
}
