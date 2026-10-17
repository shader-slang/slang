// Regression test for shader-slang/slang#12785 (status-returning buffer loads are not repeatable).
//
// `StructuredBuffer.Load(index, out status)` lowers to `StructuredBufferLoadStatus`, which reads
// the element (operand 0, an immutable resource) but ALSO writes the residency status through an
// out-param, so it is a side-effecting instruction. A wrapper around it must not be treated as a
// repeatable read-only access merely because its resource operand is immutable: the status write
// (and, under aliasing-via-binding, a subsequent read of the aliased location) means the two calls
// are not interchangeable. The gate rejects it because the load reports a side effect.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

StructuredBuffer<uint> inputData;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadWithStatus(uint index)
{
    uint status = 0;
    uint value = inputData.Load(index, status);
    return value + status;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadWithStatus(index);
    uint b = loadWithStatus(index);
    outputData[index] = a + b;
}
// Both calls survive (the definition reads `loadWithStatus_0(uint ...`, not `loadWithStatus_0(index`).
// CHECK-COUNT-2: loadWithStatus_0(index
// CHECK-NOT: loadWithStatus_0(index
