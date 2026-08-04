//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7aa52338315f8b3f3a3143cb7fd0b2de1d8d38f1996437c8b61f57df78d0402e
//META: purpose=Fires diagnostic E31222 (invalid-hlsl-register-name-for-type) -- invalid HLSL register name for type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31222
//META: catalog_name=invalid-hlsl-register-name-for-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

Texture2D tex : register(t0, space0);
RWTexture2D<float4> rwtex : register(t0);
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31222
