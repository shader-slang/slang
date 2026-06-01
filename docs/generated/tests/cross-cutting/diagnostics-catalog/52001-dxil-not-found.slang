//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E52001 (dxil-not-found) - dxil library not found
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=52001
//META: catalog_name=dxil-not-found
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Targeting dxil on a host without the dxcompiler shared library
// produces the dxil-not-found warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target dxil -entry main -profile cs_6_0

[numthreads(1,1,1)]
void main() {}

// CHECK: E52001
