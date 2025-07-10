//TEST:SIMPLE(filecheck=SPIRV): -entry VertexMain -stage vertex -target spirv -preserve-params
//TEST:SIMPLE(filecheck=SPIRV): -entry VertexMain -stage vertex -target spirv -emit-spirv-directly -preserve-params

// Test for https://github.com/shader-slang/slang/issues/7689
// SV_VertexID combined with -preserve-params should not crash

// SPIRV-DAG: BuiltIn VertexIndex
// SPIRV-DAG: BuiltIn InstanceIndex

[shader("vertex")]
float4 VertexMain(uint vertexId: SV_VertexID, uint instanceId: SV_InstanceID) : SV_Position
{
    return float4(vertexId + instanceId);
}