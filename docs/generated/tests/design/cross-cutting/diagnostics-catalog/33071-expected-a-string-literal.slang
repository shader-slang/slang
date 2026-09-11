//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c299c5b2731c9bfb1c6bc839035b23b20b957328c58f636cc11f4b8dc972df53
//META: purpose=Fires diagnostic E33071 (expected-a-string-literal) - expected a string literal
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33071
//META: catalog_name=expected-a-string-literal
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[shader(...)]` names its stage with a string literal. Passing an integer
// makes the attribute argument fail its literal-kind check, and the CHECK pins
// the expected-a-string-literal error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute
[shader(1)]
void main() {}

//CHECK: E33071
