//TEST:COMPARE_COMPUTE(targets=all):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;


[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    float v = 0.0;
    v.x = float(dispatchThreadID.x);
    outputBuffer[dispatchThreadID.x] = v.x;
}