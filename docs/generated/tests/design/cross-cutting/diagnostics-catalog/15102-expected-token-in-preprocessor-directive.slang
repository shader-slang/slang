//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=600ae451a71327a66f79f4c6ca4c7e68aa7e6f1c4d085bbb2fa445a7e26f9e58
//META: purpose=Fires diagnostic E15102 (expected-token-in-preprocessor-directive) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15102
//META: catalog_name=expected-token-in-preprocessor-directive
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15102.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define
void main() {}

//CHECK: E15102
