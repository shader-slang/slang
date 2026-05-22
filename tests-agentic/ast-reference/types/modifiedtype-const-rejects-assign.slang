//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ast-reference/types.md#existentials-and-this-types
//META: doc_section_digest=cb36cdb24311d8e6f855e3a900c470a4bebea610d6ab53b559d45e07b085b44d
//META: purpose=A ModifiedType `const T` qualifies the base type with a modifier; writes through a const-modified local are not l-values and are diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ModifiedType` wraps a base type with one or more modifiers. The
// `const` modifier removes l-value-ness from the binding; assigning
// to a const-modified local is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    const int x = 42;
    x = 1;
/*CHECK:
      ^ left of '=' is not an l-value
*/
// CHECK: E30011
}
