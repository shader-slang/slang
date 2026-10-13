//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=245694b0f86baa93e81254d7d38bec0a70bbb03c1defa221673ef6947497b8e9
//META: purpose=Fires diagnostic E15613 (pragma-warning-unknown-specifier) - unknown specifier
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15613
//META: catalog_name=pragma-warning-unknown-specifier
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The specifier before the id list in `#pragma warning` comes from a fixed set
// (push/pop/disable/enable/error). A name outside it is diagnosed, which the
// CHECK pins by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#pragma warning(bogusspecifier: 30081)
void main() {}

//CHECK: E15613
