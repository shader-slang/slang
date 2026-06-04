//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31149 (custom-derivative-signature-mismatch-at-position) -- custom derivative parameter type mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31149
//META: catalog_name=custom-derivative-signature-mismatch-at-position
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[BackwardDerivativeOf(orig)]
void derivFunc(inout DifferentialPair<int> p, float dOut) {}
[Differentiable]
float orig(float x) { return x; }


// CHECK: E31149
