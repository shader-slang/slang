//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30122 (forward-reference-in-generic-default-initializer) - forward reference in generic default initializer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30122
//META: catalog_name=forward-reference-in-generic-default-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic value parameter default initializer that references a later parameter is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<let A : int = B, let B : int>() {}

void main() {}

// CHECK: 30122
