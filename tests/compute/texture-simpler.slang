//TEST(smoke,compute):COMPARE_COMPUTE_EX:-cpu -compute  -shaderobj -output-using-type
//TEST(smoke,compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(smoke,compute):COMPARE_COMPUTE_EX:-slang -compute -dx12  -shaderobj -output-using-type
//TEST(smoke,compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -profile cs_6_0 -shaderobj -output-using-type
//TEST(smoke,compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type -render-feature hardware-device
//TEST(smoke,compute):COMPARE_COMPUTE_EX:-cuda -compute  -shaderobj -output-using-type
//TEST(smoke,compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(smoke):SIMPLE(filecheck=NVVM): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -
// NVVM: tex.level.2d.v4.f32.f32
//TEST(smoke,compute):COMPARE_COMPUTE:-slang -shaderobj -mtl -output-using-type
//TEST(smoke,compute):COMPARE_COMPUTE:-wgpu -slang -compute -shaderobj -output-using-type
// Not supported in LLVM: textures are not currently available
//DISABLE_TEST(smoke,compute):COMPARE_COMPUTE: -slang -shaderobj -llvm

//TEST_INPUT: Texture2D(size=4, content = one):name t2D
Texture2D<float> t2D;
//TEST_INPUT: Sampler:name samplerState
SamplerState samplerState;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    float u = i * 0.25;
    outputBuffer[i] = t2D.SampleLevel(samplerState, float2(u, u), 0);
}
