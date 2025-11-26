//TEST:SIMPLE(filecheck=CHECK): -target cuda -stage compute -entry computeMain
// Test int64_t atomic operations on CUDA
//CHECK: atomicAdd

RWStructuredBuffer<int64_t> buffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int64_t oldValue;
    // This should compile without errors and generate atomicAdd call
    InterlockedAdd(buffer[0], 1, oldValue);

    // Also test without the old value output
    InterlockedAdd(buffer[1], 42);
}
