//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b81b1ab5f8145f24d4a0fa519b44b3998811fdeacacbd31e6a169a7ee12ce084
//META: purpose=Fires diagnostic E15104 (expected-2-tokens-in-preprocessor-directive) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15104
//META: catalog_name=expected-2-tokens-in-preprocessor-directive
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15104.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#line abc
void main() {}

//CHECK: E15104
