//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30314 (maybe-differentiable-on-non-interface-requirement) - [MaybeDifferentiable] cannot be applied to non-interface requirement
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30314
//META: catalog_name=maybe-differentiable-on-non-interface-requirement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// [MaybeDifferentiable] applied to a normal function (not an interface requirement)
// triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[MaybeDifferentiable]
float f(float x) { return x; }

void main() {}

// CHECK: 30314
