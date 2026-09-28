//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=acce700818ea2c3c663186325cdb74dae79651532cb0acae847ae25bfb00caba
//META: purpose=Fires diagnostic E15503 (invalid-token-paste-result) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15503
//META: catalog_name=invalid-token-paste-result
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15503.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define C 1 ## $$
void main() { C; }

//CHECK: E15503
