//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=091a86c7451a6984a06615cdfdcfefc3b7597884a8c83f7aea0258b233a68f6b
//META: purpose=Fires diagnostic E38024 (invalid-dispatch-thread-id-type) - invalid SV_DispatchThreadID type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38024
//META: catalog_name=invalid-dispatch-thread-id-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a float vector type for SV_DispatchThreadID instead of
// uint/int vector triggers E38024.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

[numthreads(1,1,1)]
void main(float3 tid : SV_DispatchThreadID) {}

//CHECK: E38024
