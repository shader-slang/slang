//TEST:COMPARE_COMPUTE(targets=all):

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int i = 0;
    [ForceUnroll]
    while (i < 5 && dispatchThreadID.x == 0)
    {
        if (i >= 3)
            break;
        outputBuffer[i] = i;
        i++;
    }
}
