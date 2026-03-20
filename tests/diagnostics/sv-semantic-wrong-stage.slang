//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Test that SV_Depth is only valid in fragment shaders.
// Using it in a vertex shader should produce an error.

[shader("vertex")]
float4 main(out float depth : SV_Depth) : SV_Position
//CHECK:              ^^^^^ system value semantic 'SV_Depth' cannot be used as output in 'vertex' shader stage
{
    depth = 0.5;
    return float4(0, 0, 0, 1);
}
