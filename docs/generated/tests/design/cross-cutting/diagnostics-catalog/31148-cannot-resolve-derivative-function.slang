//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=239d4a444aef94a5c462d450823a67c259b6e81be9a849181f196bde97897305
//META: purpose=Fires diagnostic E31148 (cannot-resolve-derivative-function) -- cannot resolve derivative function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31148
//META: catalog_name=cannot-resolve-derivative-function
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
[BackwardDerivative(unknownDeriv)]
float f(float x) { return x; }


//CHECK: E31148
