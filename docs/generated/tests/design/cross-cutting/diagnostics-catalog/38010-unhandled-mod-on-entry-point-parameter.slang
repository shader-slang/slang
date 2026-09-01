//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8f8a04da6f8bf8c63263994e44ca9820e4b1d867a01c657d2cdb8884c167e83e
//META: purpose=Fires diagnostic E38010 (unhandled-mod-on-entry-point-parameter) - modifier on entry point parameter is unsupported
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38010
//META: catalog_name=unhandled-mod-on-entry-point-parameter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying `register(...)` to an entry-point parameter is
// unsupported and triggers E38010.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage vertex -entry main

float4 main(uniform float4 v : register(c0)) : SV_Position
{
    return v;
}

//CHECK: E38010