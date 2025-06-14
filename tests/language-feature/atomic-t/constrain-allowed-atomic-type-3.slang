//IGNORE:TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

// Not validated.
__generic<T>
struct NestedStruct
    where noncopyable(T)
{
    T val;
    __init()
    {
        val = T();
    }
}

__generic<T>
void logic()
    where noncopyable(T)
{
     NestedStruct<T> val = NestedStruct<T>();
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    logic<Atomic<float>>();
}
