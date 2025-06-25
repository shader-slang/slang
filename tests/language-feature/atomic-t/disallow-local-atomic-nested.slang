//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

struct NestedStruct
{
    Atomic<int> atomicBuffer;
}

struct TopStruct
{
    __init() {}
    NestedStruct inner;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
// CHECK: error 30078
    TopStruct myStruct = TopStruct();
    outputBuffer[0] = myStruct.inner.atomicBuffer.load();
}
