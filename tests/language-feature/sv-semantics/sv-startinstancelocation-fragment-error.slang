//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 30702

[shader("fragment")]
float4 main(
    uint baseInstance : SV_StartInstanceLocation)
    : SV_Target
{
    return float4(baseInstance, 0, 0, 1);
}


