//TEST:SIMPLE(filecheck=CHECK-SPIRV): -target spirv -stage fragment
//TEST:SIMPLE(filecheck=CHECK-GLSL): -target glsl -stage fragment

// Test for SPV_NV_shading_rate support (NV_FragmentSize and NV_InvocationsPerPixel)

struct FragmentInput
{
    int2 fragmentSize : NV_FragmentSize;
    int invocationsPerPixel : NV_InvocationsPerPixel;
}

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    // CHECK-SPIRV: OpCapability ShadingRateNV
    // CHECK-SPIRV: OpExtension "SPV_NV_shading_rate"
    // CHECK-SPIRV-DAG: OpDecorate {{.*}} BuiltIn FragmentSizeNV
    // CHECK-SPIRV-DAG: OpDecorate {{.*}} BuiltIn InvocationsPerPixelNV

    // CHECK-GLSL: #extension GL_NV_shading_rate_image : require
    // CHECK-GLSL-DAG: flat in ivec2 gl_FragmentSizeNV
    // CHECK-GLSL-DAG: flat in int gl_InvocationsPerPixelNV

    return float4(input.fragmentSize.x, input.fragmentSize.y, input.invocationsPerPixel, 1.0);
}