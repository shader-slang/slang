//TEST:COMPARE_COMPUTE(targets=all-llvm):-output-using-type
//TEST(smoke):COMPARE_COMPUTE(targets=dx12):-profile cs_6_0 -output-using-type
//TEST(smoke):COMPARE_COMPUTE(targets=vk):-output-using-type -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=metal+wgpu):-output-using-type
// Not supported in LLVM: textures are not currently available

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
