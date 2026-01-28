//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30703

// Test that SV_PointCoord is rejected in vertex shader (fragment-only)

[shader("vertex")]
void main(
    float2 pointCoord : SV_PointCoord,
    out float4 position : SV_Position)
{
    position = float4(pointCoord, 0, 1);
}


