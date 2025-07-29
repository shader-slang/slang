//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment

// Test NV_FragmentSize builtin semantic

[shader("fragment")]
float4 main(int2 fragmentSize : NV_FragmentSize) : SV_Target
{
    // CHECK: OpCapability ShadingRateNV
    // CHECK: OpExtension "SPV_NV_shading_rate"
    // CHECK: OpDecorate %{{.*}} BuiltIn FragmentSizeNV
    
    return float4(fragmentSize.x, fragmentSize.y, 0.0, 1.0);
}