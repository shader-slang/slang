// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK): -slang -compute
// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK): -vk -compute

// POSITIVE_CHECK tests that a user can declare/use nested concrete types in
// their type-coerce-constraint, propegating type-coerce-constraint properties

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

ToType logic<ToType, FromType>(FromType dataIn)
    where ToType == CustomStruct2
    where ToType(FromType)
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
    var t = logic(dataIn);
    outputBuffer[0] = t.getData();
}