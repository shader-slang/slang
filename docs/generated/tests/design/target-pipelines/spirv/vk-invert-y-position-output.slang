//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=dca3c920e1b8f6cdd533706f0eec3f8f5360fb308a4a71a0d356a7e7e95a99ed
//META: purpose=invertYOfPositionOutput fires under -fvk-invert-y for a vertex entry point — the SV_Position output's Y component is negated before being written to gl_Position, observable as an OpFNegate / OpFSub on the y lane in the emitted SPIR-V.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the option-set toggle VulkanInvertY gates the
// invertYOfPositionOutput pass (Phase C, row 23). With
// -fvk-invert-y on a vertex entry point, the SV_Position output
// has its Y negated so that the same Slang/HLSL coordinate system
// targets Vulkan's flipped clip space. The emitted SPIR-V still
// declares the Vertex execution model and the gl_Position BuiltIn,
// but the y-component is no longer the raw constant the user wrote.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex -fvk-invert-y

// The y-coordinate input is read from a uniform so constant folding
// cannot precompute the negation away. Without this, an
// all-literal `float4(...)` collapses to a `ConstantComposite` and
// the documented runtime OpFNegate never appears in the assembly.
uniform float y;

[shader("vertex")]
float4 main(uint vid : SV_VertexID) : SV_Position
{
    return float4(0.0, y, 0.0, 1.0);
}

// CHECK-DAG: OpEntryPoint Vertex %main
// CHECK-DAG: OpDecorate %gl_Position BuiltIn Position
// CHECK-DAG: OpFNegate
