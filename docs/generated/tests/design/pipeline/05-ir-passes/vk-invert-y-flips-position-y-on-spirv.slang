//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#target-specific-lowering
//META: doc_section_digest=557b5025d6a79f84b833cef681013d91b8a66b0574857ec5bf62d3232bf54ca2
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
