//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E49999 (unknown-system-value-semantic) - unknown system-value semantic
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=49999
//META: catalog_name=unknown-system-value-semantic
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A made-up SV_ semantic name is not in the system-value table
// and is rejected at code-generation time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main(float x : SV_BOGUSNAME) {}

// CHECK: E49999
