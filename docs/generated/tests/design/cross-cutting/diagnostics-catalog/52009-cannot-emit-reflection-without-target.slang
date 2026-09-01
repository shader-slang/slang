//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a63c1be00eede0d7198ea9525a1505997fdb8766fe04d4d61440fc4a6d45b0cf
//META: purpose=Fires diagnostic E52009 (cannot-emit-reflection-without-target) - cannot emit reflection JSON
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=52009
//META: catalog_name=cannot-emit-reflection-without-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// -reflection-json without a -target option leaves the layout
// pipeline with no target to compute against; reflection emit fails.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -reflection-json -

[numthreads(1,1,1)]
void main() {}

//CHECK: E52009
