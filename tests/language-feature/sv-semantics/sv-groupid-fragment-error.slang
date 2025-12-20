//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 38028

[shader("fragment")]
float4 main(
    uint3 groupID : SV_GroupID)
    : SV_Target
{
    return float4(groupID, 1);
}


