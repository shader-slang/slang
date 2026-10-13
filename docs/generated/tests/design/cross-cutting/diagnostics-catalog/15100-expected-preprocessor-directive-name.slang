//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c22c6e1829961ee925d54214d45c6f1c776b474a8d4750a39c944aea58a0e8e9
//META: purpose=Fires diagnostic E15100 (expected-preprocessor-directive-name) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15100
//META: catalog_name=expected-preprocessor-directive-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15100.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#@
void main() {}

//CHECK: E15100
