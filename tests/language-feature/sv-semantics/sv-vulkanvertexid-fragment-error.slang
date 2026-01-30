//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 30702

[shader("fragment")]
float4 main(
    int vertexID : SV_VulkanVertexID)
    : SV_Target
{
    return float4(vertexID, 0, 0, 1);
}


