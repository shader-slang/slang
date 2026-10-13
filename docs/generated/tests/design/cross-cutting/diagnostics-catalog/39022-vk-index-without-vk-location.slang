//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5563991187a9a66d420726d25a4d24ee2834c947c895957de9e76235f49cc1e7
//META: purpose=Fires diagnostic E39022 (vk-index-without-vk-location) - vk::index without vk::location
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39022
//META: catalog_name=vk-index-without-vk-location
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[[vk::index(N)]]` without a corresponding `[[vk::location(N)]]`
// is ignored and triggers E39022.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage fragment -entry main

[shader("fragment")]
float4 main([[vk::index(1)]] float4 col : COLOR) : SV_Target
{
    return col;
}

//CHECK: E39022