//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#existentials-and-this-types
//META: doc_section_digest=b3770682eb824e71f9cd11f91fe74a0b78caa156ada7e2c07450eae26d20b679
//META: purpose=A ModifiedType `const T` qualifies the base type with a modifier; writing through a const-modified local is not an l-value and is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies ModifiedType wraps a base type with a modifier: the
// `const` modifier removes l-value-ness, so assigning to a
// const-modified local is rejected. CHECK pins the not-an-l-value
// diagnostic (E30011).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    const int x = 42;
    x = 1;
/*CHECK:
      ^ left of '=' is not an l-value.
*/
}
