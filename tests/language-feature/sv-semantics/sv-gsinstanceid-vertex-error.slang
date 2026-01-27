//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30703

// Test that SV_GSInstanceID is rejected in vertex shader (geometry/hull-only)

[shader("vertex")]
void main(
    int gsInstance : SV_GSInstanceID,
    out float4 position : SV_Position)
{
    position = float4(gsInstance, 0, 0, 1);
}


