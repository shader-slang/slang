// gather-texture2darray.slang

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly -render-feature hardware-device
//TEST(compute):SIMPLE(filecheck=HLSL):-target hlsl -profile cs_6_6 -entry computeMain
//TEST(compute):SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute

// Test atomics on a RWTexture2D<float>.

//TEST_INPUT: set t = RWTexture2D(format=R32Float, size=4, content = zero, mipMaps = 1)
[format("r32f")]
globallycoherent RWTexture2D<float> t;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float originalValue;
    
    // HLSL: {{.*}}originalValue{{.*}} = NvInterlockedAddFp32({{.*}}t{{.*}}, {{.*}}, {{.*}}1.0{{.*}});
    // MTL: atomic_fetch_add
    t.InterlockedAddF32(uint2(1, 0), 1.0, originalValue);
    
    AllMemoryBarrier();

    // CHECK: 4.0
    outputBuffer[0] = t[uint2(1, 0)] + originalValue;

}
