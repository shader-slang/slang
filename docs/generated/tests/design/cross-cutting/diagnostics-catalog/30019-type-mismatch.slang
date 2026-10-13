//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=72ffe778cb00717e39fbbd393cb340c63dd3002cfddf50b2a64064e805d41409
//META: purpose=Fires diagnostic E30019 (type-mismatch) - type mismatch in expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30019
//META: catalog_name=type-mismatch
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Initialization only succeeds when the initializer converts to the declared
// type. A 3-component vector does not convert to a scalar `int`, and the CHECK
// pins the type-mismatch error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = float3(1,2,3); }

//CHECK: E30019
