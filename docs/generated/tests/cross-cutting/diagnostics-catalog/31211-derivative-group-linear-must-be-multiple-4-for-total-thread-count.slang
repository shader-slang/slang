//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31211 (derivative-group-linear-must-be-multiple-4-for-total-thread-count) -- derivative group linear thread count error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31211
//META: catalog_name=derivative-group-linear-must-be-multiple-4-for-total-thread-count
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeGroupLinear][shader("compute")][numthreads(3,1,1)]
void main() {}


// CHECK: E31211
