//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 38028

[shader("fragment")]
float4 main(
    int gsInstance : SV_GSInstanceID)
    : SV_Target
{
    return float4(gsInstance, 0, 0, 1);
}


