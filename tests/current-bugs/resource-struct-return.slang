//DISABLE_TEST:SIMPLE:-target hlsl -entry computeMain -profile cs_6_2

// This test demonstrates returning struct with resource causes internal compiler error

RWTexture1D<float> g_t;
RWStructuredBuffer<int> outputBuffer;

struct Thing
{
    int a;
    RWTexture1D<float> t;
};

Thing makeThing()
{
 	Thing t;
    t.a = 10;
    t.t = g_t;
    return t;
}

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = dispatchThreadID.x;
    Thing thing = makeThing();
    outputBuffer[dispatchThreadID.x] = x + thing.t.Load(1); 
}