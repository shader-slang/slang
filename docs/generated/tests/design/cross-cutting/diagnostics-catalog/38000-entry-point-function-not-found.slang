//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=20324e33a25121fb9407d942364b5adfbef806b1dead7211585b46631a03a476
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

//CHECK: E38000
