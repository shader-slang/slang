//TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DPASS
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error
// CHECK_FAIL: error 38043
__generic<T>
struct Foo
#ifdef PASS
    where noncopyable(T)
#else
    where noncopyable(int)
#endif
{
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
}
