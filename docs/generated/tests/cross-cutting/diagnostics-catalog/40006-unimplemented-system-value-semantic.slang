//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E40006 (unimplemented-system-value-semantic) - unknown system-value semantic
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40006
//META: catalog_name=unimplemented-system-value-semantic
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A bogus `SV_*` semantic name triggers E40006.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage vertex -entry main -profile sm_5_0

float4 main(float4 p : SV_BogusSemantic) : SV_Position
{
    return p;
}

// CHECK: E40006
