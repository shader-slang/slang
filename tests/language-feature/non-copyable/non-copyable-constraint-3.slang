//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_TYPE_CONSTRAINT -DNON_COPYABLE_PARAM
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DNON_COPYABLE_PARAM

//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DWITH_TYPE_CONSTRAINT
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
struct AlwaysCopyableType<T>
#ifdef WITH_TYPE_CONSTRAINT
    where noncopyable(T)
#endif
{
    // we do not use the type T in the struct to
    // ensure that the constraint is never
    // adding an implicit NonCopyable attribute
    // to the type
    int inner;
    __init(int v) { inner = v; }
}

void foo<T>(T t)
    where noncopyable(T)
{
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
#ifdef NON_COPYABLE_PARAM
    foo(AlwaysCopyableType<NonCopyableType>(1));
#else
    foo(AlwaysCopyableType<CopyableType>(1));
#endif
}
