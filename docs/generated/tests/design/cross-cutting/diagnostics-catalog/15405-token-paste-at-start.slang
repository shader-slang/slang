//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b7cca464c174e8ad8001645f1ea3c164fc1ed951b4533e31d6a260cc8c83fa44
//META: purpose=Fires diagnostic E15405 (token-paste-at-start) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15405
//META: catalog_name=token-paste-at-start
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15405.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M ## a
void main() { M; }

//CHECK: E15405
