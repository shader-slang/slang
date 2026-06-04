//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W39029 (register-modifier-but-no-vk-binding-nor-shift) - D3D register without Vulkan binding or shift
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39029
//META: catalog_name=register-modifier-but-no-vk-binding-nor-shift
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Compiling a parameter that has a `register(...)` semantic but
// no `[[vk::binding(...)]]` (and no `-fvk-...-shift`) for SPIR-V
// triggers W39029.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

Texture2D<float> tex : register(t0);

[numthreads(1,1,1)]
void main() {}

// CHECK: W39029
