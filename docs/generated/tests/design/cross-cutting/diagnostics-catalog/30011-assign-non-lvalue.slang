//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0e0a27e778bf0f89bf0f6a96b5277b973d80397f37038ad9cdc04378125a7a06
//META: purpose=Fires diagnostic E30011 (assign-non-lvalue) - left of '=' is not an l-value
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30011
//META: catalog_name=assign-non-lvalue
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Assignment requires an l-value on the left. A literal has no storage, so the
// CHECK pins the not-an-l-value error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { 1 = 2; }

//CHECK: E30011
