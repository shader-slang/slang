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

RWStructuredBuffer<int> outputBuffer;

// CHECK: error 30078
TopStruct myStruct = TopStruct();

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = myStruct.inner.atomicBuffer.load();
}
