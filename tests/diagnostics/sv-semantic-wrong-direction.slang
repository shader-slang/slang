//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that SV_VertexID can only be used as an input.
// Using it as an output should produce an error.

// CHECK: ([[# @LINE+2]]): error 40014: system-value semantic 'SV_VertexID' cannot be used as output
[shader("vertex")]
float4 main(out uint vid : SV_VertexID) : SV_Position
{
    vid = 0;
    return float4(0, 0, 0, 1);
}
