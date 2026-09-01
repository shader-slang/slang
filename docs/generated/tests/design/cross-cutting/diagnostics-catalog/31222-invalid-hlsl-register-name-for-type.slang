//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5ddaac6ccbb580709f7ee73613ad9c8e981bfbbad09e7a82489256dec8f89920
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


//CHECK: E31222
