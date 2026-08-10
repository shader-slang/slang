//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=36e91a686cf01bcb4eaea17e43fa1db17d1db02420c7afbaec2233c319b8259c
//META: purpose=Fires diagnostic E15206 (expected-integral-version-number) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15206
//META: catalog_name=expected-integral-version-number
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15206.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#version not_an_int
void main() {}

//CHECK: E15206
