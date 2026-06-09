//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=336e7d6d0383a588312b935e96dce0831d84f29a9b3a39256ea8cbd3f3840bd4
//META: purpose=Fires diagnostic E31212 (only-one-of-derivative-group-linear-or-quad-can-be-set) -- conflicting derivative group settings
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31212
//META: catalog_name=only-one-of-derivative-group-linear-or-quad-can-be-set
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeGroupLinear][DerivativeGroupQuad][shader("compute")][numthreads(8,1,1)]
void main() {}


// CHECK: E31212
