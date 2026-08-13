//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e1c1daa38d71ab9bb7d6875e8b7b574752f07aeaa28f6f21f50a14c8ca3cfe75
//META: purpose=Fires diagnostic E31212 (only-one-of-derivative-group-linear-or-quad-can-be-set) -- conflicting derivative group settings
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31212
//META: catalog_name=only-one-of-derivative-group-linear-or-quad-can-be-set
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeGroupLinear][DerivativeGroupQuad][shader("compute")][numthreads(8,1,1)]
void main() {}


//CHECK: E31212
