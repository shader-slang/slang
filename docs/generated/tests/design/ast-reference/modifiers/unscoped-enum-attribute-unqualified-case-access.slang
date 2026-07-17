//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:51+00:00
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels
//META: doc_section_digest=5c5a0f355c34a9303ec2cc1c18ffbb9a6f1d4bcd8bac23383e808da27fa7169c
//META: purpose=`[UnscopedEnum]` written by the user on a plain enum makes its cases resolvable without qualification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `UnscopedEnumAttribute` is attached from the user-written `[UnscopedEnum]`
// attribute on an enum. Its observable role is scoping: the enum's cases
// become visible in the enclosing scope, so `Red` / `Blue` resolve without
// the `Color.` prefix. Printing the values through the unqualified names
// proves each name bound to the right case.

//TEST:INTERPRET(filecheck=CHECK):

[UnscopedEnum]
enum Color
{
    Red = 7,
    Blue = 42,
}

void main()
{
    //CHECK: red=7 blue=42
    printf("red=%d blue=%d\n", int(Red), int(Blue));
}
