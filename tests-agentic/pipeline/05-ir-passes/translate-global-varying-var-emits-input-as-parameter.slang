//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=The translate-global-varying-var pass routes `SV_DispatchThreadID` into a target-shaped stage-input on every text target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Translate global varying var" row of the layout-and-binding
// table (`slang-ir-translate-global-varying-var.cpp`) routes
// stage-varying parameters into the target-shaped entry-point
// signature. `SV_DispatchThreadID` shows up as the
// `SV_DispatchThreadID` semantic on HLSL, `gl_GlobalInvocationID` on
// GLSL, `GlobalInvocationId` on SPIR-V, `global_invocation_id` on
// WGSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = tid.x;
}

// HLSL: SV_DispatchThreadID
// GLSL: gl_GlobalInvocationID
// SPIRV: GlobalInvocationId
// WGSL: global_invocation_id
