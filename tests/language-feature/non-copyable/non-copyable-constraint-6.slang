//TEST(compute):SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

// CHECK: OpEntryPoint
// CHECK-NOT: error

[vk::push_constant] int* inputBuffer;
RWStructuredBuffer<int> outputBuffer;

[__NonCopyableType]
struct NonCopyableType
{
    __init(int v) { x = v; }
    int x;
}

__generic<T>
struct NewType
{
    __init(){}
    T* ptr = nullptr;
}
__generic<T>
void foo(T* v)
{
};

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    NewType<int> newType;
    foo(inputBuffer);
}
