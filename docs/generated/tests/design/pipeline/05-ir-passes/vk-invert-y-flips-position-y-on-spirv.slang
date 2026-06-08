//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
//META: purpose=The vk-invert-y pass inserts a Y-flip on vertex-shader output position when targeting Vulkan / SPIR-V; the SPIR-V emit shows OpFNegate of the Y component.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Vulkan invert Y" row of the target-specific lowering table
// (`slang-ir-vk-invert-y.cpp`) negates the Y component of the
// vertex-shader output position when running for SPIR-V. The
// SPIR-V text shows `OpFNegate` on the Y lane near `Position`.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage vertex -entry main -fvk-invert-y

uniform RWStructuredBuffer<float> buf;

struct VOut { float4 pos : SV_Position; }

VOut main(uint vid : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(vid), float(vid) * 0.5, 0.0, 1.0);
    return o;
}

// SPIRV: OpEntryPoint Vertex
// SPIRV: OpFNegate
