//DISABLE_TEST:SIMPLE:-target hlsl -entry computeMain -profile cs_6_2

// This test demonstrates out parameter with a struct & resource type crashes 
    
RWTexture1D<float> g_t;

RWStructuredBuffer<int> outputBuffer;

struct Thing
{
    int a;
    RWTexture1D<float> t;
};

void setThing(out Thing t)
{
    t.a = 10;
    t.t = g_t;
}

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = dispatchThreadID.x;
   
    Thing thing;
    setThing(thing);
        
    outputBuffer[dispatchThreadID.x] = x + thing.t.Load(1); 
}
