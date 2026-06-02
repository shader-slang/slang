//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5564328489fb0769a5142651d7ec41cf34dd5aade70763c9a42892c1a5f8b30b
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


// CHECK: E31148
