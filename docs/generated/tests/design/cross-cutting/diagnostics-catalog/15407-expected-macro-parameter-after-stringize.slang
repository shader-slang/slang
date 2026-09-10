//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7f7331317b7f3d98294a51c4943f5712cf6dc9c5d797fc05add409c49b5a0fb5
//META: purpose=Fires diagnostic E15407 (expected-macro-parameter-after-stringize) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15407
//META: catalog_name=expected-macro-parameter-after-stringize
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15407.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M(a) # b
void main() { M(1); }

//CHECK: E15407
