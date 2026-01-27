//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that SV_IsFrontFace can only be used as an input in fragment shader.
// Using it as an output from vertex shader should produce an error.

// CHECK: ([[# @LINE+2]]): error 30703: system value semantic 'SV_IsFrontFace' cannot be used as output in 'vertex' shader stage
[shader("vertex")]
float4 main(out bool frontFace : SV_IsFrontFace) : SV_Position
{
    frontFace = true;
    return float4(0, 0, 0, 1);
}
