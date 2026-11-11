//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6d3d353f7acb30664cc90831179bf06d5f613a7e643d2f70a23548558a35ee35
//META: purpose=Attribute argument checking rejects a non-string-literal argument to `[shader(...)]` (E33071).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "attribute argument types" rule. The stage attribute takes a
// string naming the stage; an integer literal cannot be interpreted as one, so
// the argument is rejected during modifier checking rather than being coerced.
// CHECK pins E33071 on the argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

[shader(3)]
void main()
{
}
//CHECK: E33071
