//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39007 (unknown-register-class) - unknown register class
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39007
//META: catalog_name=unknown-register-class
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using an unrecognized HLSL register class prefix (e.g. `q`)
// triggers E39007.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

Texture2D<float> tex : register(q0);

[numthreads(1,1,1)]
void main() {}

// CHECK: E39007
