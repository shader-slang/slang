//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -xslang -zero-initialize
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -emit-spirv-directly -allow-glsl -xslang -zero-initialize
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -entry computeMain -allow-glsl -xslang -zero-initialize
//TEST(smoke,compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-dx12 -use-dxil -compute -entry computeMain -allow-glsl -xslang -zero-initialize

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum MyFlags1
{
    Zero = 0,
    One,
}

enum MyFlags2 : uint8_t
{
    Zero = 0,
    One,
}

enum MyFlags3 : uint16_t
{
    Zero = 0,
    One,
}

enum MyFlags4 : uint32_t
{
    Zero = 0,
    One,
}

enum MyFlags5 : uint64_t
{
    Zero = 0,
    One,
}

int getDefaultInt()
{
    int val;
    return val;
}
float getDefaultFloat()
{
    float val;
    return val;
}
double getDefaultDouble()
{
    double val;
    return val;
}
vector<int, 2> getDefaultIntVec2()
{
    vector<int, 2> val;
    return val;
}
vector<int, 3> getDefaultIntVec3()
{
    vector<int, 3> val;
    return val;
}
vector<int, 4> getDefaultIntVec4()
{
    vector<int, 4> val;
    return val;
}
[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int reg1 = getDefaultInt();
    float reg2 = getDefaultFloat();
    double reg3 = getDefaultDouble();
    vector<int, 2> vectorVal1 = getDefaultIntVec2();
    vector<int, 3> vectorVal2 = getDefaultIntVec3();
    vector<int, 4> vectorVal3 = getDefaultIntVec4();
// BUF: 1
    outputBuffer[0] = true
    && reg1 == 0
    && reg2 == 0
    && reg3 == 0
    && vectorVal1 == vector<int, 2>(0)
    && vectorVal2 == vector<int, 3>(0)
    && vectorVal3 == vector<int, 4>(0)
    && MyFlags1::One == MyFlags1(1)
    && MyFlags2::One == MyFlags2(1)
    && MyFlags3::One == MyFlags3(1)
    && MyFlags4::One == MyFlags4(1)
    && MyFlags5::One == MyFlags5(1)
    ;
}
