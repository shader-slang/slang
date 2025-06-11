//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv -DFAIL

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error
// CHECK_FAIL: error 39999
// Foo should implicitly be non-copyable if var of type T is 
// defined since T is `noncopyable`

__generic<T>
struct Foo
    where noncopyable(T)
{
    __init(){}
#ifdef FAIL
    T v;
#endif
};

RWStructuredBuffer<int> outputBuffer;

__generic<T>
void test(T inputVal) 
{
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    test(Foo<int>());
}
