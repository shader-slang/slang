//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A ModifiedType applies a modifier to a base type; a `const`-modified local is not an l-value, so assigning to it is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modifier really participates in the type rather
// than being dropped after parsing: a `const int` local still accepts
// its initialiser but refuses a later assignment. CHECK pins the
// not-an-l-value diagnostic, which a plain `int` would not produce.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    const int x = 42;
    x = 1;
/*CHECK:
      ^ left of '=' is not an l-value.
*/
}
