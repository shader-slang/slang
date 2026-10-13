//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=20673cb4358c7ef2dc6197cbe8ab1af517130201709c40bc3eba85147e09d1b1
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

//CHECK: E39028
