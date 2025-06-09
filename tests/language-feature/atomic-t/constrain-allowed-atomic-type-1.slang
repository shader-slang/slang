//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -DCASE_1
//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -DCASE_2
//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -DCASE_3
//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -DCASE_4
//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -DCASE_5

RWStructuredBuffer<int> outputBuffer;

// CHECK: error 30079

#if defined(CASE_1)
ConstantBuffer<Atomic<int>> atomicBuffer;
#elif defined(CASE_2)
StructuredBuffer<Atomic<int>> atomicBuffer;
#elif defined(CASE_3)
StructuredBuffer<StructuredBuffer<Atomic<int>>> atomicBuffer;
#elif defined(CASE_4)
__generic<T>
struct Thing
    where noncopyable(T)
{
    T val;
    __init(float val)
    {
    }
};
RWStructuredBuffer<Thing<Atomic<int>>> atomicBuffer;
#elif defined(CASE_5)
__generic<T>
public struct TestAtomicNested
{
    T val;
    __init(float val)
    {
    }
};

__generic<T>
public struct TestAtomic 
    where noncopyable(T)
{
    T val;
    __init(float val)
    {
    }
};

RWStructuredBuffer<TestAtomicNested<TestAtomic<Atomic<int>>>> test_atomic_buffer;
#endif

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    atomicBuffer;
}
