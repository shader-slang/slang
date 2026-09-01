//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=59a5d96a48ff1c6d0214b63f170480dc2200e4b770d2d41c86a76ab01f6d3f4f
//META: purpose=Fires diagnostic E15406 (token-paste-at-end) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15406
//META: catalog_name=token-paste-at-end
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15406.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M a ##
void main() { M; }

//CHECK: E15406
