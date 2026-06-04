//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W38052 (vertex-shader-missing-sv-position) - vertex shader has no output with the 'SV_Position' system value semantic
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38052
//META: catalog_name=vertex-shader-missing-sv-position
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A vertex shader with no SV_Position output triggers W38052.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage vertex -profile sm_5_0

[shader("vertex")]
float4 main() : COLOR
{
    return float4(0);
}

// CHECK: W38052
