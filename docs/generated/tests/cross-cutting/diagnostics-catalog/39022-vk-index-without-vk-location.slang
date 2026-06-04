//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W39022 (vk-index-without-vk-location) - vk::index without vk::location
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39022
//META: catalog_name=vk-index-without-vk-location
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[[vk::index(N)]]` without a corresponding `[[vk::location(N)]]`
// is ignored and triggers W39022.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage fragment -entry main

[shader("fragment")]
float4 main([[vk::index(1)]] float4 col : COLOR) : SV_Target
{
    return col;
}

// CHECK: W39022
