//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

// Debug updates must follow the resource/ordinary field split through nested aggregates.
// The texture has no function-local debug storage, while the UV and marker fields still do.
//TEST_INPUT: Texture2D(size=4, content = zero):name tex
Texture2D<float4> tex;
SamplerState samp;
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4, count=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Inner
{
    float2 uv;
    Texture2D<float4> texture;
};

struct Outer
{
    Inner inner;
    uint marker;
};

[numthreads(2, 2, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Outer value;
    value.inner.uv = float2(tid.xy) / 2.0;
    value.marker = 1;
    bool sampleTexture = tid.x != 0;
    if (sampleTexture)
        value.inner.texture = tex;

    float result = float(value.marker);
    if (sampleTexture)
        result = value.inner.texture.SampleLevel(samp, value.inner.uv, 0).x;
    outputBuffer[tid.x + tid.y * 2] = result;
    // BUF: 1.000000
    // BUF: 0.000000
    // BUF: 1.000000
    // BUF: 0.000000
}
