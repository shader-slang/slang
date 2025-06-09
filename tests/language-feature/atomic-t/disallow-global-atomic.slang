//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<float> outputBuffer;

// CHECK: error 30078
static Atomic<int> atomicBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    atomicBuffer;
}
