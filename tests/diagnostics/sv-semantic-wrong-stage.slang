//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that SV_Depth is only valid in fragment shaders.
// Using it in a vertex shader should produce an error.

// CHECK: ([[# @LINE+2]]): error 30702: system value semantic 'SV_Depth' cannot be used as output in 'vertex' shader stage
[shader("vertex")]
float4 main(out float depth : SV_Depth) : SV_Position
{
    depth = 0.5;
    return float4(0, 0, 0, 1);
}
