//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK_1): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK_2): -vk -shaderobj

//TEST_INPUT:ubuffer(data=[1 0 0 0], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// POSITIVE_CHECK_1 tests that `implicit` type coerce constraints validate parameters correctly.
//
// POSITIVE_CHECK_2 tests that type coerce constraints validate parameters correctly.

#ifdef POSITIVE_CHECK_1
int castableConstraint1<ToType, FromType>(FromType input) where ToType(FromType) implicit
#else
int castableConstraint1<ToType, FromType>(FromType input) where ToType(FromType)
#endif
{
    return inputBuffer[0];
}

struct CustomStruct1
{
    int data;
    __init(int other)
    {
        data = other;
    }
}

struct CustomStruct2
{
    int data;

#ifdef POSITIVE_CHECK_1
    __implicit_conversion(400)
#endif
    __init(CustomStruct1 input)
    {
        data = input.data;
    }

    __init(int other)
    {
        data = other;
    }
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    //POSITIVE_CHECK_1: 1
    //POSITIVE_CHECK_2: 1
    outputBuffer[0] = (true
                 && castableConstraint1<CustomStruct2, CustomStruct1>(CustomStruct1(1)) == 1
                 && castableConstraint1<CustomStruct2, CustomStruct2>(CustomStruct2(1)) == 1
                ) ? 1 : 0;
}