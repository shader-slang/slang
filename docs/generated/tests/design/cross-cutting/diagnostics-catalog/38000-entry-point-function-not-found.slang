//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38000 (entry-point-function-not-found) - no function found matching entry point name
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38000
//META: catalog_name=entry-point-function-not-found
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Requesting an entry-point name that doesn't exist in the source
// triggers E38000.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry doesNotExist -stage compute

[numthreads(1,1,1)]
void main() {}

// CHECK: E38000
