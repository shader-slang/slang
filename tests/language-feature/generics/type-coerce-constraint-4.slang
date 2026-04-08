// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK): -slang -compute
// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK): -vk -compute

// POSITIVE_CHECK tests that the compile works since FromType can be converted to ToType
// via the info the type-coerce-constraint propagates (`where ToType(FromType)`) from the `logic` method.

// TEST_INPUT:ubuffer(data=[2 3 0 0], stride=4)name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct CustomStruct1
{
    int data = 1;
    __init(int other)
    {
        data = other;
    }
    int getData()
    {
        return data;
    }
}
struct CustomStruct2
{
    int data = 1;
    __init(CustomStruct1 other)
    {
        data = other.getData()+inputBuffer[1];
    }
    int getData()
    {
        return data;
    }
}

ToType castableConstraint<ToType, FromType>(FromType dataIn) where ToType(FromType)
{
    return ToType(dataIn);
}

ToType logic<ToType, FromType>(FromType dataIn) where ToType(FromType)
{
    return castableConstraint<ToType, FromType>(dataIn);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // POSITIVE_CHECK: 5
    int data = inputBuffer[0];
    CustomStruct1 dataIn = CustomStruct1(data);
    var t = logic<CustomStruct2, CustomStruct1>(dataIn);
    outputBuffer[0] = t.getData();
}