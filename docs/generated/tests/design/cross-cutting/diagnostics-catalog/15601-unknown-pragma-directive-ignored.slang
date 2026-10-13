//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a653ff0a32b300a204d69668afd4d712bc976dd66cade72a803fa4811ee635b7
//META: purpose=Fires diagnostic E15601 (unknown-pragma-directive-ignored) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15601
//META: catalog_name=unknown-pragma-directive-ignored
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15601.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#pragma unknown_pragma_xyz
void main() {}

//CHECK: E15601
