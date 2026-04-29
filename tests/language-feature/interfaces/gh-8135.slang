// Regression test for https://github.com/shader-slang/slang/issues/8135
// Crash when using associated types from inherited generic interfaces.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Parent interface declares an associated type.
public interface IStorage<T>
{
    public associatedtype Address;
}

// Child interface inherits from the parent and uses the inherited
// associated type in its method signatures. This previously caused a
// compiler crash with "assert failure: parentNonBlock".
public interface IParameterStorage<T> : IStorage<T>
{
    public void store(Address address, T value);
    public Address resolveAddress(T value);
}

// Concrete implementation satisfies both interfaces.
struct DenseStorage : IParameterStorage<int>
{
    typedef uint Address;

    void store(uint address, int value) {}

    uint resolveAddress(int value)
    {
        return uint(value < 0 ? 0 : value);
    }
}

// Generic function that uses the inherited associated type through the interface.
void testStorage<T, S : IParameterStorage<T>>(S storage, T val)
{
    S.Address addr = storage.resolveAddress(val);
    storage.store(addr, val);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 1
// CHECK: 0
// CHECK: 0
// CHECK: 0

[numthreads(4, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    DenseStorage s;
    testStorage(s, tid.x);
    outputBuffer[tid.x] = (tid.x == 0) ? 1 : 0;
}
