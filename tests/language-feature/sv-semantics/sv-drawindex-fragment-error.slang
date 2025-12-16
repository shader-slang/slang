//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 38028

[shader("fragment")]
float4 main(
    uint drawID : SV_DrawIndex)
    : SV_Target
{
    return float4(drawID, 0, 0, 1);
}


