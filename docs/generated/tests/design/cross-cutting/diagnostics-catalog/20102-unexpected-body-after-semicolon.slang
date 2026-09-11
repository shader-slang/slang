//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=47504d885976a3a18c251ba2820168062782a113c3cb6b726670f2e0ee40ebc3
//META: purpose=Fires diagnostic E20102 (unexpected-body-after-semicolon) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20102
//META: catalog_name=unexpected-body-after-semicolon
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E20102.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void foo(); { }
void main() {}

//CHECK: E20102
