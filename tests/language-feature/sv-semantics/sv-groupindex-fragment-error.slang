//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 40015

[shader("fragment")]
float4 main(
    uint groupIndex : SV_GroupIndex)
    : SV_Target
{
    return float4(groupIndex, 0, 0, 1);
}


