//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint32_t> outputBuffer;

interface myType
{
};

[UnscopedEnum]
enum unscopedEnum : myType
{
    kEnumVal1 = 1
};

struct structWithEnumVal
{
    unscopedEnum val;
};


[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 5
    outputBuffer[0] = 5;
}