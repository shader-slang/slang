//TEST:SIMPLE(filecheck=CHECK-SPIRV): -target spirv -stage fragment -entry main
//TEST:SIMPLE(filecheck=CHECK-GLSL): -target glsl -stage fragment -entry main

// Test for SPV_EXT_fragment_invocation_density support (SV_FragSize and SV_FragInvocationCount)

struct FragmentInput
{
    int2 fragmentSize : SV_FragSize;
    int invocationsPerPixel : SV_FragInvocationCount;
}

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    // CHECK-SPIRV: OpCapability FragmentDensityEXT
    // CHECK-SPIRV: OpExtension "SPV_EXT_fragment_invocation_density"
    // CHECK-SPIRV-DAG: OpDecorate {{.*}} BuiltIn FragSizeEXT
    // CHECK-SPIRV-DAG: OpDecorate {{.*}} BuiltIn FragInvocationCountEXT

    // CHECK-GLSL: #extension GL_EXT_fragment_invocation_density : require
    // CHECK-GLSL-DAG: gl_FragSizeEXT
    // CHECK-GLSL-DAG: gl_FragInvocationCountEXT

    return float4(input.fragmentSize.x, input.fragmentSize.y, input.invocationsPerPixel, 1.0);
}