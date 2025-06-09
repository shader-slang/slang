//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT -DNON_COPYABLE_PARAM
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DNON_COPYABLE_PARAM

//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_CONSTRAINT
//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv

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

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error
// CHECK_FAIL: error 38042
void foo<T>(T t)
#ifdef WITH_CONSTRAINT
    where noncopyable(T)
#endif
{
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
#ifdef NON_COPYABLE_PARAM
    foo(NonCopyableType(1));
#else
    foo(CopyableType(1));
#endif
}
