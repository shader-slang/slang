//TEST:COMPARE_COMPUTE(targets=vk+dx12+cuda+cpu+llvm, filecheck-buffer=BUF):-entry computeMain

#pragma warning(disable:30816)


//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct DefaultStructNoInit_base
{
    static const int data0 = 2;
    static int data1 = 2;
    int data2 = 2;
};

struct DefaultStructNoInit : DefaultStructNoInit_base
{
    int data3 = 2;
    static int data4 = 2;
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DefaultStructNoInit::data4 = 0;
    DefaultStructNoInit noInit = {};
    // BUF: 1
    outputBuffer[0] = true
        && noInit.data0 == 2
        && noInit.data1 == 2
        && noInit.data2 == 2
        && noInit.data3 == 2
        && noInit.data4 == 0;
        ;
}
