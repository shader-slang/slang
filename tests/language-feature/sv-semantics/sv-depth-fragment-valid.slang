//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: OpEntryPoint

// Test that SV_Depth works correctly in fragment shader

[shader("fragment")]
void main(
    out float depth : SV_Depth,
    out float4 color : SV_Target)
{
    depth = 0.5;
    color = float4(1, 0, 0, 1);
}


