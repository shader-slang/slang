//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2488500952a63cd6d9d3a59367c44b82cf8e45b7c74627a3a1e788e79dde3a3e
//META: purpose=Fires diagnostic E50011 (spirv-version-not-supported) - SPIR-V version too old
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=50011
//META: catalog_name=spirv-version-not-supported
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Slang's direct SPIR-V backend only emits 1.3+. Requesting a 1.0
// profile triggers the too-old-version warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute -profile spirv_1_0

[numthreads(1,1,1)]
void main() {}

//CHECK: E50011
