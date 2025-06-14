// TEST(compute):SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv
// TEST(compute) : SIMPLE(filecheck = CHECK) : -stage compute - entry computeMain - target spirv -DFAIL_NONCOPYABLE_TUPLE

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


// CHECK-NOT: OpEntryPoint
// CHECK: error
__generic<each T>
void foo(T t)
{
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
#ifdef FAIL_NONCOPYABLE_TUPLE
    var tuple = Tuple<NonCopyableType, CopyableType, CopyableType>( NonCopyableType(0), CopyableType(1), CopyableType(2));
#else
    var tuple = Tuple<CopyableType, CopyableType, CopyableType>( CopyableType(0), CopyableType(1), CopyableType(2));
#endif
    foo(tuple);
}
