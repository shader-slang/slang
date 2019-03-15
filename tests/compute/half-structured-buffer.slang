//TEST(compute):COMPARE_COMPUTE:-dx12 -compute -use-dxil -profile cs_6_2
//DISABLE_TEST(compute):COMPARE_COMPUTE:-vk -compute -profile cs_6_2
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):dxbinding(0),glbinding(0),out

struct Thing
{
    uint pos;
    half2 color;
    int b;
    float radius;
};

RWStructuredBuffer<Thing> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    
    float v = tid * (1.0f / 3.0f);
    
    Thing thing;
    thing.pos = tid;
    thing.color = half2(v, v);
    thing.b = -(int)tid;
    
    //thing.color = half4(v, 0, 0, v);
    thing.radius = v;
    
    outputBuffer[tid] = thing;
}