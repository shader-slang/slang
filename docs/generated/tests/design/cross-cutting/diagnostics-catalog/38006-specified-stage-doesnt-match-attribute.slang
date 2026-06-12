//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W38006 (specified-stage-doesnt-match-attribute) - entry point stage mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38006
//META: catalog_name=specified-stage-doesnt-match-attribute
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// -stage vertex on the command line but [shader("fragment")] on
// the entry point triggers W38006.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage vertex -entry main -profile sm_5_0

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(1.0f);
}

// CHECK: W38006
