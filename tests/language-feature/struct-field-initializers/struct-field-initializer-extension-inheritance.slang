//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain 
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain
//TEST(smoke,compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -use-dxil -compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct DefaultStructNoInit_base
{
    int data0 = 2;
    int data1 = 2;
};
extension DefaultStructNoInit_base
{
    __init(int indata)
    {
        //unused
    }
}
struct DefaultStructNoInit : DefaultStructNoInit_base
{
    int dataInherit = 2;
};

struct DefaultStructWithInit_base
{
    int data0;
    int data1 = 3;
};
extension DefaultStructWithInit_base
{
    __init(int indata)
    {
        //unused
    }
    __init()
    {
        data0 = 3;
    }
}
struct DefaultStructWithInit : DefaultStructWithInit_base
{
    int dataInherit = 3;
};

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DefaultStructNoInit noInit = DefaultStructNoInit();
    DefaultStructWithInit withInit = DefaultStructWithInit();
    // BUF: 1
    outputBuffer[0] = true
        && noInit.data0 == 2
        && noInit.data1 == 2
        && noInit.dataInherit == 2

        && withInit.data0 == 3
        && withInit.data1 == 3
        && withInit.dataInherit == 3
        ;
}