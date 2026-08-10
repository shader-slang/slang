//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
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
