//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<int> outputBuffer;

// CHECK: OpEntryPoint
// CHECK-NOT: error
RWStructuredBuffer<Atomic<int>> atomicBuffer1;
GLSLShaderStorageBuffer<Atomic<int>> atomicBuffer2;
[[vk::push_constant]] Atomic<int>* atomicBuffer3;
StructuredBuffer<Ptr<Atomic<int>>> atomicBuffer4;
groupshared Atomic<int> atomicBuffer5;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    atomicBuffer1;
    atomicBuffer2;
    atomicBuffer3;
    atomicBuffer4;
    atomicBuffer5;
}