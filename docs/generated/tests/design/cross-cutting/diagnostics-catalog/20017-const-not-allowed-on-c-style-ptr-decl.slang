//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2971703cdb2abc31d80a4bdb1dfc26ad7eef7522b57b10bbc28267ac09963fc5
//META: purpose=Fires diagnostic E20017 (const-not-allowed-on-c-style-ptr-decl) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20017
//META: catalog_name=const-not-allowed-on-c-style-ptr-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E20017.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
const int* p;
void main() {}

//CHECK: E20017
