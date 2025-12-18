//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

//TEST_INPUT: Texture2D(size=4, format=D32Float, content = one):name texture
Texture2D texture;
//TEST_INPUT: Sampler(depthCompare):name sampler
SamplerComparisonState sampler;

//TEST_INPUT: ubuffer(data=[0], stride=1):out,name output
RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void computeMain()
{
    // METALLIB: sample_compare_depth_2d
    // BUF: 1.000000
    output[0] = texture.SampleCmpLevelZero(sampler, float2(0, 0), 0);
}
