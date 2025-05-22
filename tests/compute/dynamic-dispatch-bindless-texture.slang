// Test using interface typed shader parameters with texture typed fields.
//TEST(compute):COMPARE_COMPUTE:-cpu
//TEST(compute):COMPARE_COMPUTE:-cuda

// Type must be marked `public` to ensure it is visible in the generated DLL.

export struct MyImpl
{
    Texture2D tex;
    float run()
    {
        return tex.Load({0, 0, 0}).x;
    }
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<uint> gOutputBuffer;
//TEST_INPUT: set gCb = new StructuredBuffer<MyImpl>{new MyImpl{Texture2D(size=8, content = one)}}
StructuredBuffer<MyImpl> gCb;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;

    let inputVal : int = tid;
    MyImpl v0 = gCb.Load(0);
    let outputVal = v0.run();
    gOutputBuffer[tid] = uint(trunc(outputVal));
}