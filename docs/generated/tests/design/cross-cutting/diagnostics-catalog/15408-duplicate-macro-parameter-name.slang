//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8bcf5aea5de1e98841b9956f80d43cbde3651bbe27ccd13c97e4ddd8efae2330
//META: purpose=Fires diagnostic E15408 (duplicate-macro-parameter-name) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15408
//META: catalog_name=duplicate-macro-parameter-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15408.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M(a, a) a
void main() { M(1,2); }

//CHECK: E15408
