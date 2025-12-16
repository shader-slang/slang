//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 38028

[shader("fragment")]
float4 main(
    uint baseInstance : SV_StartInstanceLocation)
    : SV_Target
{
    return float4(baseInstance, 0, 0, 1);
}


