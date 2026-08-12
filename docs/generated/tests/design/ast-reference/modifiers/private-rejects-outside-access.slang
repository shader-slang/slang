//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#visibility-modifiers
//META: doc_section_digest=1c8e25b6beb8d6b322c1d0d2039a7c5fcba36699beabc8ab756c7f1c0d6fb97b
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
