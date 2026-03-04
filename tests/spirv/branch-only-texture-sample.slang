//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

//TEST_INPUT: Texture2D(size=4, content = zero):name tex1
Texture2D<float4> tex1;
SamplerState samp;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=1, count=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct S {
    float2 uv;
    Texture2D<float4> t;
};

[numthreads(2, 2, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x + dispatchThreadID.y * 2;
    float param = float(dispatchThreadID.x) / 2.0;
    float2 coord = float2(dispatchThreadID.xy) / 2.0;

    bool b = (param >= 0.5);

    S s;
    s.uv = coord;

    if (b) {
        s.t = tex1;
    }

    float4 col = float4(1, 1, 1, 1);

    if (b) {
        col = s.t.SampleLevel(samp, s.uv, 0);
    }

    outputBuffer[index] = col.x;
    // BUF: 1.000000
    // BUF: 0.000000
    // BUF: 1.000000
    // BUF: 0.000000
}

