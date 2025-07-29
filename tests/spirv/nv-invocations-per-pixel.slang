//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment

// Test NV_InvocationsPerPixel builtin semantic

[shader("fragment")]
float4 main(int invocationsPerPixel : NV_InvocationsPerPixel) : SV_Target
{
    // CHECK: OpCapability ShadingRateNV
    // CHECK: OpExtension "SPV_NV_shading_rate"
    // CHECK: OpDecorate %{{.*}} BuiltIn InvocationsPerPixelNV
    
    return float4(invocationsPerPixel, 0.0, 0.0, 1.0);
}