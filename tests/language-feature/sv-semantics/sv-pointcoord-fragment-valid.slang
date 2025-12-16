//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: OpEntryPoint

// Test that SV_PointCoord works correctly in fragment shader

[shader("fragment")]
float4 main(
    float2 pointCoord : SV_PointCoord)
    : SV_Target
{
    return float4(pointCoord, 0, 1);
}


