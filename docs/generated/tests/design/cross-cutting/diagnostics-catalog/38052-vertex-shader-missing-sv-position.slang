//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6b88c7580fdafeda692c8f982fd8bf21eda0910aa374ea4fa15e75f243d3fe15
//META: purpose=Fires diagnostic E38052 (vertex-shader-missing-sv-position) - vertex shader has no output with the 'SV_Position' system value semantic
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38052
//META: catalog_name=vertex-shader-missing-sv-position
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A vertex shader with no SV_Position output triggers E38052.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-Wpedantic -target hlsl -entry main -stage vertex -profile sm_5_0

[shader("vertex")]
float4 main() : COLOR
{
    return float4(0);
}

//CHECK: E38052