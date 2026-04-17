// Test discard statement emission across multiple targets.
// From the docs on inactive threads:
// "The thread has executed a discard statement, which disables the thread
//  (fragment shaders only)."
//
// Verifies discard emits correctly for HLSL, GLSL, and SPIRV.
// Also tests that discard can be used conditionally.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv -entry fragmentMain -stage fragment -profile spirv_1_6

uniform float threshold;

// CHECK_HLSL: discard
// CHECK_GLSL: discard
// CHECK_SPIRV: OpDemoteToHelperInvocation

[shader("fragment")]
float4 fragmentMain(float4 color : COLOR) : SV_Target
{
    // Conditional discard based on alpha
    if (color.a < threshold)
        discard;

    return color;
}
