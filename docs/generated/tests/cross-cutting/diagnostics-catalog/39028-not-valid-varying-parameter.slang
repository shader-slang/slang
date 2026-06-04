//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39028 (not-valid-varying-parameter) - not a valid varying parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39028
//META: catalog_name=not-valid-varying-parameter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A varying entry-point parameter of an unsupported type (e.g.
// matrix with dimension 1 in a pixel shader output position)
// triggers E39028.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage vertex -entry main -profile sm_5_0

interface IFoo {}

float4 main(IFoo foo : A) : SV_Position
{
    return float4(0);
}

// CHECK: E39028
