//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=9bb14d8a0f3d89541f8a2b039ef21e5fa84bd3dcbbe45aabd27eab7f34cfae12
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

//CHECK: E57004
