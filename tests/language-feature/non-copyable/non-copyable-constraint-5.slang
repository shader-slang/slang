//TEST(compute):SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

interface IFoo
{
    __init(int v);
}

[__NonCopyableType]
struct NonCopyableType : IFoo
{
    __init(int v) { x = v; }
    int x;
}

struct CopyableType : IFoo
{
    __init(int v) { x = v; }
    int x;
}


// CHECK: OpEntryPoint
// CHECK-NOT: error
__generic<each T>
void foo(T t)
    where noncopyable(T)
{
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    var tuple = makeTuple(NonCopyableType(0), CopyableType(1), CopyableType(2));
    foo(tuple);
}
