//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=b4c8d3637c56a76592380b5850772aac25face32ea591824cb9bbf293ce90b35
//META: purpose=Verifies SV_VertexID/SV_InstanceID lower to gl_VertexIndex/gl_InstanceIndex with the gl_BaseVertex/gl_BaseInstance offset subtracted in emitted GLSL.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins a non-obvious GLSL legalization detail: HLSL's SV_VertexID and
// SV_InstanceID are zero-based per-draw, but Vulkan's gl_VertexIndex /
// gl_InstanceIndex include the draw's base offset. The GLSL system-var
// storage path (slang-ir-glsl-legalize.cpp) compensates by emitting
// `gl_VertexIndex - gl_BaseVertex` and `gl_InstanceIndex - gl_BaseInstance`
// so the user value keeps HLSL semantics. CHECK pins both subtractions.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage vertex

struct VOut { float4 p : SV_Position; uint id : VID; }

[shader("vertex")]
VOut main(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VOut o;
    o.p = float4(float(vid));
    o.id = vid + iid;
    return o;
}

// SV_VertexID is rebased against gl_BaseVertex.
//GLSL-DAG: gl_VertexIndex - gl_BaseVertex
// SV_InstanceID is rebased against gl_BaseInstance.
//GLSL-DAG: gl_InstanceIndex - gl_BaseInstance
