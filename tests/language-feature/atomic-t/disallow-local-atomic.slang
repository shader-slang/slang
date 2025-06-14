//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
// CHECK: error 30078
    Atomic<int> atomicBuffer;
    outputBuffer[0] = atomicBuffer.load();
}
