//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bc6a1d1e33b5d749c15546c394714e68777913d94f58c326b72dddd78897f8da
//META: purpose=Fires diagnostic E15409 (variadic-macro-parameter-must-be-last) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15409
//META: catalog_name=variadic-macro-parameter-must-be-last
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15409.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M(a..., b) a
void main() { M(1,2); }

//CHECK: E15409
