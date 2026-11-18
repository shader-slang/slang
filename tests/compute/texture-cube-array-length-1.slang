// A TextureCube input with an explicit arrayLength of 1 is a single-element
// cube array: it must be created as an array texture so the resource view type
// matches the TextureCubeArray type the shader declares, even though one cube
// is also a valid plain cube shape.
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: TextureCube(size=4, content=one, arrayLength=1):name tCubeArray
TextureCubeArray<float4> tCubeArray;

//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = int((tCubeArray.SampleLevel(samplerState, float4(0, 0, 0, 0), 0)).x);
    outputBuffer[0] = 0x12345 + ret;
}
// CHECK: 12346
