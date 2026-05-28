// Regression test for https://github.com/shader-slang/slang/issues/8135.
// Using an associated type from an inherited generic interface in a child
// interface method signature previously crashed in hoistable IR placement.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

public interface IStorage<T>
{
    public associatedtype Address;
}

public interface IParameterStorage<T> : IStorage<T>
{
    public void add(Address address, T value);
    public Address getAddress(T value);
}

struct DenseStorage : IParameterStorage<int>
{
    typedef uint Address;

    void add(uint address, int value)
    {}

    uint getAddress(int value)
    {
        return uint(value);
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
    testStorage(storage, int(dispatchThreadID.x));
}

// CHECK: OpEntryPoint GLCompute
// CHECK: OpExecutionMode {{.*}} LocalSize 1 1 1
// CHECK: OpReturn
