//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#visibility-modifiers
//META: doc_section_digest=0b85270cdc11d14a0cd871e0cb80fb27c01d6be160703736facd597770df0b42
//META: purpose=A `private` member is not accessible from outside its enclosing type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `PrivateModifier` restricts visibility to the enclosing type
// declaration. Accessing a `private` field from outside is diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    private int secret;
}

void main()
{
    S s;
    int v = s.secret;
    //CHECK: error
    //CHECK: E30600
}
