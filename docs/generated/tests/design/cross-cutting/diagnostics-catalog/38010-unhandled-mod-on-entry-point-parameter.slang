//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W38010 (unhandled-mod-on-entry-point-parameter) - modifier on entry point parameter is unsupported
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38010
//META: catalog_name=unhandled-mod-on-entry-point-parameter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying `register(...)` to an entry-point parameter is
// unsupported and triggers W38010.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage vertex -entry main

float4 main(uniform float4 v : register(c0)) : SV_Position
{
    return v;
}

// CHECK: W38010
