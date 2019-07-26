//TEST:CPP_COMPILER_COMPILE: -profile cs_5_0 -entry computeMain -target cpp

struct Thing
{
    int a;
    float b;
};

struct AnotherThing
{
    Texture2D<float> tex;
    SamplerState sampler;
};

ParameterBlock<AnotherThing> s_paramBlock;

//ConstantBuffer<Thing> thing;

ConstantBuffer<Thing> thing3;

[numthreads(4, 1, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID, 
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
    RWStructuredBuffer<int> outputBuffer, 
    uniform Thing thing, 
    uniform Thing thing2)
{
    uint tid = dispatchThreadID.x;

    float v = s_paramBlock.tex.Load(int3(tid, tid, 0));
   
    outputBuffer[tid] = int(tid * tid) + thing.a + thing3.a + int(v); // + thing.a;
}