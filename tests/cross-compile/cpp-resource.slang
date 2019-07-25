//TEST:CPP_COMPILER_COMPILE: -profile cs_5_0 -entry computeMain -target cpp

struct Thing
{
    int a;
    float b;
};

//ConstantBuffer<Thing> thing;
//Texture2D<float> tex;

[numthreads(4, 1, 1)]
void computeMain(
    uint3 dispatchThreadID : SV_DispatchThreadID, 
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
    RWStructuredBuffer<int> outputBuffer, 
    uniform Thing thing)
{
    uint tid = dispatchThreadID.x;

    outputBuffer[tid] = int(tid * tid) + thing.a; // + thing.a;
}