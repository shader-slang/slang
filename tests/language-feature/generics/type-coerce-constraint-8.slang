// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK): -slang -compute
// TEST(compute):COMPARE_COMPUTE(filecheck-buffer=POSITIVE_CHECK): -vk -compute

// POSITIVE_CHECK tests that Associated type works as expected with Type Coercion Constraints

// TEST_INPUT:ubuffer(data=[1 2 3 0], stride=4)name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

// TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface Data
{
    int getData();
}
struct DataImpl : Data
{
    int data;
    __init(int other)
    {
        data = other + 1;
    }
    int getData()
    {
        return data;
    }
}

interface MaybeCastInt
{
    int getData();
}

struct MaybeCastIntImpl1 : MaybeCastInt
{
    int data;
    __init<T : Data>(T other)
    {
        data = other.getData() + 1;
    }
    int getData()
    {
        return data;
    }
}
struct MaybeCastIntImpl2 : MaybeCastInt
{
    int data;
    __init<T : Data>(T other)
    {
        data = other.getData() + 2;
    }
    int getData()
    {
        return data;
    }
}

interface CustomStruct<U : Data>
{
    associatedtype T
        where T : MaybeCastInt
        where T(U);
}

  struct CustomStructImpl1<U : Data> : CustomStruct<U>
{
    typealias T = MaybeCastIntImpl1;
}
struct CustomStructImpl2<U : Data> : CustomStruct<U>
{
    typealias T = MaybeCastIntImpl2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // POSITIVE_CHECK: 4
    DataImpl loadedData = DataImpl(inputBuffer[1]);
    outputBuffer[0] = CustomStructImpl1<DataImpl>().T(loadedData).getData();
}