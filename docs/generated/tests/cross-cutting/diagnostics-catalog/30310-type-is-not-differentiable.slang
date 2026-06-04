//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30310 (type-is-not-differentiable) - type is not differentiable
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30310
//META: catalog_name=type-is-not-differentiable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-differentiable `int` parameter in a differentiable function triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
int f(int x) { return x * x; }

void main() {}

// CHECK: 30310
