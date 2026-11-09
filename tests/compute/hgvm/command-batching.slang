//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[shader("compute")]
[numthreads(1, 1, 1)]
void storeValue(RWStructuredBuffer<uint> output, uint value)
{
    output[0] = value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void addValue(RWStructuredBuffer<uint> output, uint value)
{
    output[0] += value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void copyAndAdd(
    RWStructuredBuffer<uint> output,
    RWStructuredBuffer<uint> input,
    uint value)
{
    output[0] = input[0] + value;
}

[host]
void main()
{
    RWStructuredBuffer<uint> intermediate = RWStructuredBuffer<uint>::alloc(1);
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(1);

    // These dependent operations stay in one command encoder. The global barriers between them
    // make the second and third shaders observe the preceding unordered-access writes.
    dispatch(storeValue, uint3(1, 1, 1))(intermediate, 10);
    dispatch(addValue, uint3(1, 1, 1))(intermediate, 5);
    dispatch(copyAndAdd, uint3(1, 1, 1))(output, intermediate, 7);
    print(readBack(output, 0));

    // Readback closes the first batch. Later dispatches must start a new encoder and still retain
    // their ordering until the second readback closes that batch.
    dispatch(addValue, uint3(1, 1, 1))(output, 3);
    dispatch(addValue, uint3(1, 1, 1))(output, 4);
    print(readBack(output, 0));

    // Overwriting `temporary` drops the old buffer's final program reference before submission.
    // DeviceState's pending holds must keep it alive for both encoded operations.
    RWStructuredBuffer<uint> temporary = RWStructuredBuffer<uint>::alloc(1);
    RWStructuredBuffer<uint> lifetimeOutput = RWStructuredBuffer<uint>::alloc(1);
    dispatch(storeValue, uint3(1, 1, 1))(temporary, 33);
    dispatch(copyAndAdd, uint3(1, 1, 1))(lifetimeOutput, temporary, 0);
    temporary = RWStructuredBuffer<uint>::alloc(1);
    print(readBack(lifetimeOutput, 0));

    // Leave one final batch without a readback. Normal invocation completion must submit it
    // without waiting, and interpreter shutdown must safely drain its in-flight resource holds.
    dispatch(storeValue, uint3(1, 1, 1))(temporary, 41);
    temporary = RWStructuredBuffer<uint>::alloc(1);
}

// CHECK: 22
// CHECK-NEXT: 29
// CHECK-NEXT: 33
