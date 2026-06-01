//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39023 (mixing-implicit-and-explicit-binding-for-varying-params) - mixing implicit and explicit varying bindings
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39023
//META: catalog_name=mixing-implicit-and-explicit-binding-for-varying-params
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Mixing an explicit `[[vk::location(...)]]` and an implicit
// (no binding) varying in the same struct triggers E39023.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage vertex -entry main

struct VSIn
{
    [[vk::location(0)]] float4 a : A;
    float4 b : B;
}

float4 main(VSIn input) : SV_Position
{
    return input.a + input.b;
}

// CHECK: E39023
