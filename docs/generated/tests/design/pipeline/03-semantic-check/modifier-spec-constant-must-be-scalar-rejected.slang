//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6d3d353f7acb30664cc90831179bf06d5f613a7e643d2f70a23548558a35ee35
//META: purpose=Modifier validation rejects `[SpecializationConstant]` on a value whose type is neither scalar nor enum (E31218).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an attribute-plus-declared-type rule rather than a pure placement
// rule: the attribute is legal on a constant, but only when that constant's
// type is scalar or enum, and a vector fails that test. CHECK pins E31218 on
// the attribute.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[SpecializationConstant]
const float3 v = float3(1, 2, 3);
//CHECK: E31218

void main()
{
}
