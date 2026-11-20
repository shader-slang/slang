//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=BUF):-entry computeMain

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct DefaultStructWithInit
{
    int data0 = 3;
    int data1 = 0;
    __init(int inData)
    {
        data1 = 3;
    }
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DefaultStructWithInit withInit = DefaultStructWithInit(5);
    // BUF: 1
    outputBuffer[0] = true
        && withInit.data0 == 3
        && withInit.data1 == 3
        ;
}
