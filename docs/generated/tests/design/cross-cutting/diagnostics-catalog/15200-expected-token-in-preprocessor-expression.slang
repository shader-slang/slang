//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=dc7631ce46f39fde61fb4a6bb8bbe053dbda8cb478df738390002b19cc39ab5d
//META: purpose=Fires diagnostic E15200 (expected-token-in-preprocessor-expression) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15200
//META: catalog_name=expected-token-in-preprocessor-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15200.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if (1+
#endif
void main() {}

//CHECK: E15200
