//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2d5b49cb0a5f8c3330c06c5511e4614037b71d539232561f5ad3cdf56c153b52
//META: purpose=Fires diagnostic E30009 (invalid-type-void) - invalid type 'void'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30009
//META: catalog_name=invalid-type-void
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `void` is not a storable type, so it cannot be the type of a variable. The
// CHECK pins the invalid-void-type error that rejects the declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { void v; }

//CHECK: E30009
