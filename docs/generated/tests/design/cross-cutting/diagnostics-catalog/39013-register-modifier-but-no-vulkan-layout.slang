//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c634860b92b83a7ef4c3b355f689c78a9831e7b609a230a73f7076c0f8038c45
//META: purpose=Fires diagnostic E39013 (register-modifier-but-no-vulkan-layout) - D3D register without Vulkan binding
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39013
//META: catalog_name=register-modifier-but-no-vulkan-layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A combined Sampler2D using `register(...)` for D3D but without
// `[[vk::binding(...)]]` triggers E39013.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

Sampler2D<float4> tex : register(t0);

[numthreads(1,1,1)]
void main() {}

//CHECK: E39013