//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a9ffb05c5d3a702f0b0fd23ea81fda94c5f07e39f6b63663d3aa52d68a7562f2
//META: purpose=Fires diagnostic E15203 (expected-token-in-defined-expression) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15203
//META: catalog_name=expected-token-in-defined-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15203.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if defined(
#endif
void main() {}

//CHECK: E15203
