//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: OpEntryPoint

// Test vertex semantics work correctly

[shader("vertex")]
void main(
    uint vertexID : SV_VertexID,
    uint instanceID : SV_InstanceID,
    int vulkanVertexID : SV_VulkanVertexID,
    int vulkanInstanceID : SV_VulkanInstanceID,
    uint baseVertex : SV_StartVertexLocation,
    uint baseInstance : SV_StartInstanceLocation,
    uint drawID : SV_DrawIndex,
    out float4 position : SV_Position,
    out float pointSize : SV_PointSize)
{
    position = float4(vertexID, instanceID, 0, 1);
    pointSize = 1.0;
}


