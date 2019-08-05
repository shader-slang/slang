//TEST:CPP_COMPILER_COMPILE: -profile cs_5_0 -entry computeMain -target cpp

struct Thing
{
    int a;
    float b;
};

static int value;

// Don't use parameter block for now
//ParameterBlock<AnotherThing> s_paramBlock;

ConstantBuffer<Thing> thing3;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
RWStructuredBuffer<int> outputBuffer;

Texture2D<float> tex;
SamplerState sampler;
    
[numthreads(4, 1, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID, 
    uniform Thing thing, 
    uniform Thing thing2)
{
    uint tid = dispatchThreadID.x;
    
    float2 loc = dispatchThreadID.xy * 0.5f;

    float v = tex.Load(int3(tid, tid, 0));
    float s = tex.Sample(sampler, loc);
   
    outputBuffer[tid] = int(tid * tid) + thing.a + thing3.a + int(v + s) + value; // + thing.a;
}