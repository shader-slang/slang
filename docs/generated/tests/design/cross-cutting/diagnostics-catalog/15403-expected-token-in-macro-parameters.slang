//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8056ea340abb7f5c0cb6997e5e6b24bdc2b91fedfe88c71b2549d18e3cd09b56
//META: purpose=Fires diagnostic E15403 (expected-token-in-macro-parameters) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15403
//META: catalog_name=expected-token-in-macro-parameters
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15403.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M(a b) a+b
void main() {}

//CHECK: E15403
