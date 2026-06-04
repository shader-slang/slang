//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39008 (expected-a-register-index) - expected a register index
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39008
//META: catalog_name=expected-a-register-index
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A bare register class without a numeric index triggers E39008.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

Texture2D<float> tex : register(t);

[numthreads(1,1,1)]
void main() {}

// CHECK: E39008
