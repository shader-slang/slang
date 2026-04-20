// Test that `discard` emits correctly across HLSL, GLSL, and SPIR-V targets.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage fragment -entry fragMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry fragMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage fragment -entry fragMain -emit-spirv-directly -profile spirv_1_6

[shader("fragment")]
float4 fragMain(float4 color : COLOR) : SV_Target
{
    if (color.r < 0.5)
        discard;
    return color;
}

// CHECK_HLSL: discard
// CHECK_GLSL: discard
// CHECK_SPIRV: OpDemoteToHelperInvocation
