//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 38028

[shader("fragment")]
float4 main(
    int vertexID : SV_VulkanVertexID)
    : SV_Target
{
    return float4(vertexID, 0, 0, 1);
}


