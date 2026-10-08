//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=82e3e9c5f8d62222c8fc1783bcc0230a1d78bb61a10b791e89c2e9600996b165
//META: purpose=Fires diagnostic E38006 (specified-stage-doesnt-match-attribute) - entry point stage mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38006
//META: catalog_name=specified-stage-doesnt-match-attribute
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// -stage vertex on the command line but [shader("fragment")] on
// the entry point triggers E38006.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage vertex -entry main -profile sm_5_0

[shader("fragment")]
float4 main() : SV_Target
{
    return float4(1.0f);
}

//CHECK: E38006