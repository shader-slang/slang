//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E52009
