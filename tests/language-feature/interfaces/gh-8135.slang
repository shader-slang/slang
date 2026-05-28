// Regression test for https://github.com/shader-slang/slang/issues/8135.
// Using an associated type from an inherited generic interface in a child
// interface method signature previously crashed in hoistable IR placement.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

public interface IStorage<T>
{
    public associatedtype Address;
}

public interface IParameterStorage<T> : IStorage<T>
{
    public void add(Address address, T value);
    public Address getAddress(T value);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct DenseStorage : IParameterStorage<int>
{
    typedef uint Address;

    void add(uint address, int value)
    {
        outputBuffer[address] = value;
    }

    uint getAddress(int value)
    {
        return 0;
    }
}

void testStorage<T, S : IParameterStorage<T>>(S storage, T value)
{
    S.Address address = storage.getAddress(value);
    storage.add(address, value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    DenseStorage storage;
    // CHECK: 7
    testStorage(storage, int(dispatchThreadID.x) + 7);
}
