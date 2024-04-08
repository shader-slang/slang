//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain 
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain
//TEST(smoke,compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -use-dxil -compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static int myOne = 1;

public struct DefaultStruct_base
{
    int data0;
    int data1 = myOne;
    int data2 = 1;
    int data3 = 1;
};
struct DefaultStruct : DefaultStruct_base
{
    int data4 = 3;
    int data5 = 0;
    __init()
    {
        data3 = data3 == 1;
        data5 = 1;
        data4 = data3;
    }
};
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DefaultStruct_base initStructBase = DefaultStruct_base();
    initStructBase.data0 = 1;

    DefaultStruct initStruct = DefaultStruct();
    initStruct.data0 = 1;
    // BUF: 1
    outputBuffer[0] = true
        && initStructBase.data0 == 1
        && initStructBase.data1 == 1
        && initStructBase.data2 == 1
        && initStructBase.data3 == 1

        && initStruct.data0 == 1
        && initStruct.data1 == 1
        && initStruct.data2 == 1
        && initStruct.data3 == 1
        && initStruct.data4 == 1
        && initStruct.data5 == 1;
        ;
}