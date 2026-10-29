//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#visibility-modifiers
//META: doc_section_digest=a70c733ae597690ad884cb8c9b0fe94f72ef3bc3b77ebbe47caacff2707b1c61
//META: purpose=A `private` member is not accessible from outside its enclosing type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `private` narrows visibility to the enclosing type declaration
// rather than being an accepted no-op keyword. The field is read from a free
// function and CHECK pins the visibility error code, which only fires if the
// modifier actually restricted lookup.

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
