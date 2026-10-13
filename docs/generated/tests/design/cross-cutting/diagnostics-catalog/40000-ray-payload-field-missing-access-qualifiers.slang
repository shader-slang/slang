//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0dba11f089950a95a3158a483b7060971c403630a0071e31846524e5cbbff479
//META: purpose=Fires diagnostic E40000 (ray-payload-field-missing-access-qualifiers) - ray payload field missing access qualifiers
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40000
//META: catalog_name=ray-payload-field-missing-access-qualifiers
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A field in a `[raypayload]` struct without read/write access
// qualifiers triggers E40000.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage closesthit -entry main

struct [raypayload] Payload
{
    float c;
}

[shader("closesthit")]
void main(inout Payload p, in BuiltInTriangleIntersectionAttributes attrs)
{
    p.c = 0.0;
}

//CHECK: E40000
