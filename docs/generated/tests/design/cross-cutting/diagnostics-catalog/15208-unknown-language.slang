//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=65f74559e52d23b0393efb877d35df0e6b1ccbc166766e76da32d89a5ab5b9ce
//META: purpose=Fires diagnostic E15208 (unknown-language) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15208
//META: catalog_name=unknown-language
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15208.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#language cobol_xx
void main() {}

//CHECK: E15208
