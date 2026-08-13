//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=85f9eccdf732defa5a03b9b3664465b1a9ffc9a0344d31f04f8f61bca4e21c71
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
