//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ec790559deea93f9398a620acc1c376762a701e80383d03a3e914cf08e8d6561
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

//CHECK: E39008
