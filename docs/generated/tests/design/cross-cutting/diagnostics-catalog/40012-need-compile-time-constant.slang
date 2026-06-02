//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E40012 (need-compile-time-constant) - expected a compile-time constant
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40012
//META: catalog_name=need-compile-time-constant
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a runtime value where a compile-time constant is
// required (here as the array dimension inside a function)
// triggers E40012.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int n = tid.x;
    int arr[n];
}

// CHECK: E40012
