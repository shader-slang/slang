//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30097 (function-not-marked-as-differentiable) - function not differentiable
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30097
//META: catalog_name=function-not-marked-as-differentiable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Calling a non-`[Differentiable]` function from a differentiable function triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

float not_diff(float x) { return x * x; }

[Differentiable]
float caller(float x) { return not_diff(x); }

void main() {}

// CHECK: 30097
