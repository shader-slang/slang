//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain 
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -compute -entry computeMain
//TEST(smoke,compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -use-dxil -compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static int myTwo = 2;
static int myThree = 1+2;

struct DefaultStruct_base
{
    int data0 = 1;
    int data1;

    __init()
    {
        data1 = 1;
    }
};
struct DefaultStruct1 : DefaultStruct_base
{
    int data2 = 1;
};
struct DefaultStruct2 : DefaultStruct_base
{
    int data2 = 1;
    __init()
    {
        if (data0 != 1)
        {
            data2 = 0;
        }
    }
};
struct DefaultStruct3 : DefaultStruct_base
{
    __init()
    {
    }
};
struct DefaultStruct4 : DefaultStruct_base
{
};
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DefaultStruct1 s1 = {};
    DefaultStruct2 s2;
    DefaultStruct3 s3;
    DefaultStruct4 s4 = {};
    // BUF: 1
    outputBuffer[0] = true
        && s1.data0 == 1
        && s1.data1 == 1
        && s1.data2 == 1
        && s2.data0 == 1
        && s2.data1 == 1
        && s2.data2 == 1
        && s3.data0 == 1
        && s3.data1 == 1
        && s4.data0 == 1
        && s4.data1 == 1
        ;
}
