//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E57004 (output-spv-is-empty) - SPIR-V output contains no exported symbols
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=57004
//META: catalog_name=output-spv-is-empty
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Compiling for spirv with no entry point produces a module with
// no exported symbols; the backend rejects the empty output.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

void helper() {}

// CHECK: E57004
