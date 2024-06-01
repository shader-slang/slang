//TEST:SIMPLE(filecheck=SPV16): -target spirv -profile spirv_1_6
//TEST:SIMPLE(filecheck=SPV15): -target spirv -profile spirv_1_5

uniform int a;

// Test that we translate `discard` to OpDemoateToHelperInvocation
// when targeting SPIRV 1.6, and to OpKill when targeting SPIRV 1.5.

//SPV16: OpDemoteToHelperInvocation
//SPV15: OpKill

[shader("fragment")]
float4 frag() : SV_Target
{
    if (a == 0)
        discard;
    return 0;
}