//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W39013 (register-modifier-but-no-vulkan-layout) - D3D register without Vulkan binding
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39013
//META: catalog_name=register-modifier-but-no-vulkan-layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A combined Sampler2D using `register(...)` for D3D but without
// `[[vk::binding(...)]]` triggers W39013.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

Sampler2D<float4> tex : register(t0);

[numthreads(1,1,1)]
void main() {}

// CHECK: W39013
