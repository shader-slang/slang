// Test reading parameter block from a member function.

//TEST:COMPARE_COMPUTE(targets=vk):-output-using-type
//TEST:COMPARE_COMPUTE(targets=dx11):-profile sm_5_0 -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<int> gOutputBuffer;

struct MyParameters
{
    int x;
    int y;
    StructuredBuffer<float> buffer1;

    RWStructuredBuffer<uint3> buffer;
    int calc()
    {
        buffer[0].x = 3;
        return x + y + buffer[0].x;
    }
}

//TEST_INPUT: set gObj = new MyParameters{2, 3, new StructuredBuffer<float>{0.0}, new RWStructuredBuffer<uint3>{{0,0,0}}}
ParameterBlock<MyParameters> gObj;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float result = 0.0;
    gOutputBuffer[dispatchThreadID.x] = gObj.calc();
}
