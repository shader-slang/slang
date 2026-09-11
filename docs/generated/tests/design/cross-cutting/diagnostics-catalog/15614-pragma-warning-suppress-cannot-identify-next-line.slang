//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=08c788bc04607d8be7f6bc51b4dfceaa0a2da3f99107e7201c07db87aa4343e6
//META: purpose=Fires diagnostic E15614 (pragma-warning-suppress-cannot-identify-next-line) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15614
//META: catalog_name=pragma-warning-suppress-cannot-identify-next-line
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15614.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
//CHECK: E15614
void main() {}
#pragma warning(suppress: 41024)