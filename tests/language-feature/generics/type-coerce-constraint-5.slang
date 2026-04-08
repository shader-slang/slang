// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=POSITIVE_CHECK): -slang -compute -shaderobj
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=POSITIVE_CHECK): -vk -compute -shaderobj

// POSITIVE_CHECK tests a type-coerce-constraint in a struct generics argument list.

struct WrappedData1
{
    int data;
    __init(WrappedData2 other)
    {
        data = other.getData()+inputBuffer[1];
    }
    int getData()
    {
        return data;
    }
}

struct WrappedData2
{
    int data;
    __init(int other)
    {
        data = other;
    }
    int getData()
    {
        return data;
    }
}

struct CustomStruct1<T, U> where T(U)
{
    T data;
    __init(U other)
    {
        data = (T)other;
    }
    T getData()
    {
        return data;
    }
}

// TEST_INPUT:ubuffer(data=[2 3 0 0], stride=4)name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // POSITIVE_CHECK: 5
    int data = inputBuffer[0];
    WrappedData2 wrappedData = WrappedData2(data);
    CustomStruct1<WrappedData1, WrappedData2> processedData =
        CustomStruct1<WrappedData1, WrappedData2>(wrappedData);
    outputBuffer[1] = 1;
    outputBuffer[0] = processedData.getData().getData();
}