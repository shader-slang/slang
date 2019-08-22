//TEST(compute):COMPARE_COMPUTE:

struct Param
{
    Texture2D tex;
    SamplerState samplerState;
    float base;
};

//TEST_INPUT:Texture2D(size=4, content = one) : dxbinding(0),glbinding(0),name=diffuseMap
Texture2D diffuseMap;
//TEST_INPUT: Sampler : dxbinding(0),glbinding(0,1,2,3,4,5,6),name=samplerState
SamplerState samplerState;
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float4 run(Param p)
{
    return p.tex.SampleLevel(p.samplerState, float2(0.0), 0) + p.base;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Param p;
    p.tex = diffuseMap;
    p.samplerState = samplerState;
    p.base = -0.5;
	float4 outVal = run(p);
	
    outputBuffer[0] = outVal.x;
	outputBuffer[1] = outVal.y;
	outputBuffer[2] = outVal.z;
	outputBuffer[3] = outVal.w;
}