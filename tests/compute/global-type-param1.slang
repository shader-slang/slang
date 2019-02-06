//TEST(smoke,compute):COMPARE_COMPUTE:
//TEST_INPUT: cbuffer(data=[0.5 0 0 0 1.0], stride=4):dxbinding(0),glbinding(0)
//TEST_INPUT: cbuffer(data=[1.0], stride=4):dxbinding(1),glbinding(1)
//TEST_INPUT: Texture2D(size=4, content = zero) : dxbinding(0),glbinding(0)
//TEST_INPUT: Texture2D(size=4, content = one) : dxbinding(1),glbinding(1)
//TEST_INPUT: Sampler : dxbinding(0),glbinding(0,1,2,3,4,5,6)
//TEST_INPUT: Sampler : dxbinding(1),glbinding(0,1,2,3,4,5,6)
//TEST_INPUT: ubuffer(data=[0], stride=4):dxbinding(0),glbinding(0),out
//TEST_INPUT: type Impl

RWStructuredBuffer<float> outputBuffer;

interface IBase
{
    float compute();
}

struct Impl : IBase
{
    float base;  // = 1.0
    Texture2D tex;
    SamplerState sampler;
    float compute()
    {
        return tex.SampleLevel(sampler, float2(0.0), 0.0).x + base;
    }
};

cbuffer C
{
    float base0; // = 0.5    
}

Texture2D tex1;  // = 0.0
SamplerState sampler;

[numthreads(1, 1, 1)]
void computeMain<
    TImpl : IBase>(
    uniform ParameterBlock<TImpl> impl,
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint tid = dispatchThreadID.x;
    float b0 = tex1.SampleLevel(sampler, float2(0.0), 0.0).x + base0; // = 0.5
	float outVal = impl.compute();  // = 2.0
    outVal = b0 / outVal; // = 0.25
	outputBuffer[tid] = outVal;
}