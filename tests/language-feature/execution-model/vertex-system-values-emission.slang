// Vertex shader system value coverage: SV_VertexID, SV_InstanceID,
// SV_Position output. Each must lower to the target's built-in
// vertex-stage input/output.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage vertex -entry vertexMain -emit-spirv-directly

struct VOut
{
    float4 position : SV_Position;
    uint vid : VID;
};

[shader("vertex")]
VOut vertexMain(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    VOut o;
    o.position = float4(float(vertexId), float(instanceId), 0.0, 1.0);
    o.vid = vertexId;
    return o;
}

// HLSL keeps the literal semantics.
// CHECK_HLSL-DAG: SV_VertexID
// CHECK_HLSL-DAG: SV_InstanceID
// CHECK_HLSL-DAG: SV_Position

// GLSL uses gl_VertexIndex / gl_InstanceIndex (Vulkan) and gl_Position.
// CHECK_GLSL-DAG: gl_Position
// CHECK_GLSL-DAG: gl_VertexIndex
// CHECK_GLSL-DAG: gl_InstanceIndex

// SPIRV emits BuiltIn decorations on the entry-point inputs.
// CHECK_SPIRV-DAG: BuiltIn VertexIndex
// CHECK_SPIRV-DAG: BuiltIn InstanceIndex
// CHECK_SPIRV-DAG: BuiltIn Position
