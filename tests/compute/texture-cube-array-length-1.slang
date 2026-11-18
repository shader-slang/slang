// A TextureCube input with an explicit arrayLength of 1 is a single-element
// cube array: it must be created as an array texture so the resource view type
// matches the TextureCubeArray type the shader declares, even though one cube
// is also a valid plain cube shape. Both SPIR-V emit paths (direct and via
// GLSL) must classify the input identically, so the test runs one leg with
// -emit-spirv-via-glsl and one without.
//
// The regression guard is Vulkan debug-layer validation: a mismatched view
// type is undefined-but-often-correct on real drivers, so the sampled value
// alone does not distinguish the shapes. The CHECK value is deterministic
// because content=one fills every texel of every face, making the sampled
// value 1 regardless of which face the zero direction selects.
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-compute -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: TextureCube(size=4, content=one, arrayLength=1):name tCubeArray
TextureCubeArray<float4> tCubeArray;

//TEST_INPUT: Sampler(filteringMode=point):name samplerState
SamplerState samplerState;

[numthreads(1, 1, 1)]
void computeMain()
{
    int ret = int((tCubeArray.SampleLevel(samplerState, float4(0, 0, 0, 0), 0)).x);
    outputBuffer[0] = 0x12345 + ret;
}
// CHECK: 12346
