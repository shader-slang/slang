//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT: set g_texture = Texture2D(size=8, content = one)
//TEST_INPUT: set g_sampler = Sampler
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer

Texture2D g_texture;
SamplerState g_sampler;
RWStructuredBuffer<float> outputBuffer;

float4 sample(float2 uv, constexpr int2 ofs )
{
    return g_texture.SampleLevel(g_sampler, uv, 0.0, ofs); 
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    float2 dir = float2(1, 0);
    const int2 offset = (int2)-dir;

    float4 c = float4(0, 0, 0, 0);

    [ForceUnroll]
    for (int i = 0; i < 2; i++)
    {
        c += g_texture.SampleLevel(g_sampler, float2(0.0), 0, offset + i * dir);
    }
}
