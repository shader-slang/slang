//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=59f46b0faeb3c978ae74009c8912762dc3597142f51693ee2c7d086acd908a53
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

//CHECK: E39023
