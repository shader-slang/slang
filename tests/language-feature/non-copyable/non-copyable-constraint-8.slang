// TEST(compute):SIMPLE(filecheck=CHECK_PASS): -stage compute -entry computeMain -target spirv -DPASS
//TEST(compute):SIMPLE(filecheck=CHECK_FAIL): -stage compute -entry computeMain -target spirv

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error

// CHECK_FAIL: error 39999

RWStructuredBuffer<int> outputBuffer;

[__NonCopyableType]
struct NonCopyableType
{
    int x;
};

__generic<T>
void foo(T v)
{
};

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
#ifdef PASS
    Array<int, 3> arr = { 1, 2, 3 };
#else
    Array<NonCopyableType, 3> arr = { NonCopyableType(1), NonCopyableType(2), NonCopyableType(3) };
#endif
    foo(arr);
}
