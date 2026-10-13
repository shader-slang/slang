//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=09409799a68de63cd40d16048cdf7f065f638c190612eefcac8d930f840838e4
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


//CHECK: E31149
