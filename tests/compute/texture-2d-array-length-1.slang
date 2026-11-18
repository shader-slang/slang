// A Texture2D input with an explicit arrayLength of 1 is a single-element
// 2D array, the 2D counterpart of texture-cube-array-length-1.slang: it must
// be created as an array texture so the resource view type matches the
// Texture2DArray type the shader declares. render-test classifies the input
// once, host-side; the test runs both SPIR-V emit paths (direct and via GLSL)
// so each generated SPIR-V variant passes the Vulkan validation layer's
// checks against the array texture. The dx12 and cuda legs sample through the
// same resource on a second backend family (dx12 runs on Windows CI only).
// The CHECK value is deterministic because content=one fills every texel.
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-compute -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: Texture2D(size=4, content=one, arrayLength=1):name t2DArray
Texture2DArray<float4> t2DArray;

//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;

[numthreads(1, 1, 1)]
void computeMain()
{
    int ret = int((t2DArray.SampleLevel(samplerState, float3(0, 0, 0), 0)).x);
    outputBuffer[0] = 0x12345 + ret;
}
// CHECK: 12346
