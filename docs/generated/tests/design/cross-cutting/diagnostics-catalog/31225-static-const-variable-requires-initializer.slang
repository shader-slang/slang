//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=defa9dd80c08325d7bbce0ef0a00696af9fcd2b608b55d94ecc9c1ec5da60889
//META: purpose=Fires diagnostic E31225 (static-const-variable-requires-initializer) - missing initializer for static const
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31225
//META: catalog_name=static-const-variable-requires-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A static const global without an initializer triggers E31225.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const int g;

//CHECK: E31225
