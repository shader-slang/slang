//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31200 (deprecated-usage) -- use of deprecated declaration
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31200
//META: catalog_name=deprecated-usage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[deprecated("use newF instead")] void oldF() {}
[shader("compute")][numthreads(1,1,1)]
void main() { oldF(); }


// CHECK: E31200
