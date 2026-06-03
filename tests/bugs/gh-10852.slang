// Test that DescriptorHandle with coherent/globallycoherent type modifier compiles.
// Issue: https://github.com/shader-slang/slang/issues/10852

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -profile cs_6_6

// Verify the globallycoherent qualifier propagates through DescriptorHandle to HLSL output.
// CHECK: globallycoherent RWTexture2D

uniform DescriptorHandle<globallycoherent RWTexture2D<float4>> tex;
uniform DescriptorHandle<coherent RWTexture2D<float4>> tex2;

[numthreads(1,1,1)]
void computeMain()
{
    var t = getDescriptorFromHandle(tex);
    t[uint2(0,0)] = float4(1.0);

    var t2 = getDescriptorFromHandle(tex2);
    t2[uint2(1,0)] = float4(2.0);
}
