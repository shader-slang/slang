//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30702

// Test that SV_Depth is rejected in vertex shader (fragment-only)

[shader("vertex")]
void main(
    out float depth : SV_Depth,
    out float4 position : SV_Position)
{
    depth = 0.5;
    position = float4(0, 0, 0, 1);
}


