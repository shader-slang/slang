//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 38028

[shader("fragment")]
float4 main(
    int instanceID : SV_VulkanInstanceID)
    : SV_Target
{
    return float4(instanceID, 0, 0, 1);
}


