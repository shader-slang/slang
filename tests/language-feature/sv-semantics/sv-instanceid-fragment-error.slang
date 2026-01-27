//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// SV_InstanceID in fragment shader is valid - SPIRV doesn't support it as a builtin,
// so it's treated as a flat-interpolated user value from vertex shader.
// See commit 2407966e8 "SPIR-V SV_InstanceId support in pixel shader" and gh-3087.

// CHECK: OpEntryPoint

[shader("fragment")]
float4 main(
    uint instanceID : SV_InstanceID)
    : SV_Target
{
    return float4(instanceID, 0, 0, 1);
}

