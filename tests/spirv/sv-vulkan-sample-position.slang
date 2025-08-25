//TEST:SIMPLE(filecheck=CHECK-SPIRV): -target spirv -stage fragment -entry main
//TEST:SIMPLE(filecheck=CHECK-GLSL): -target glsl -stage fragment -entry main

// Test for SV_VulkanSamplePosition support

// CHECK-SPIRV: OpCapability SampleRateShading
// CHECK-SPIRV-DAG: OpDecorate {{.*}} BuiltIn SamplePosition
// CHECK-GLSL-DAG: gl_SamplePosition

import glsl;

struct FragmentInput
{
    float2 samplePosition : SV_VulkanSamplePosition;
}

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    return float4(input.samplePosition.x, gl_SamplePosition.y, 0.0, 1.0);
}