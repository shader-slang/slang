//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// CHECK: OpEntryPoint
// CHECK-NOT: error

[__NonCopyableType]
struct NonCopyableType
{
    __init(){}
}

T sum<T>(IArray<T> array)
    where noncopyable(T)
{
    return array[0];
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    NonCopyableType arr[3] = { NonCopyableType(), NonCopyableType(), NonCopyableType() };
    sum(arr);
}