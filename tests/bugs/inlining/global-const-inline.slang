//TEST:COMPARE_COMPUTE(targets=all):

static const float3 CONSTANT = float3(16);

int test(int value)
{
    return value*int(CONSTANT.x) + value;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let index = int(dispatchThreadID.x);
    let value = test(index);
    outputBuffer[index] = value;
}