// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api cuda

struct RetainedEmpty
{
}
struct ErasedEmpty
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValues<T>(
    uniform T first,
    uint3 tid: SV_DispatchThreadID,
    uniform uint offset,
    uniform T middle,
    uniform RWStructuredBuffer<uint> output,
    uint3 groupID: SV_GroupID,
    uniform uint value,
    uniform T last)
{
    output[offset + tid.x] = value + groupID.x;
}

// A host signature preserves the nominal empty type. All three empty arguments remain in
// bytecode, and each must resolve to its own reflected parameter without consuming a binding.
[host]
void enqueue(RetainedEmpty empty, RWStructuredBuffer<uint> output)
{
    dispatch(writeValues<RetainedEmpty>, uint3(2, 1, 1))(empty, 0u, empty, output, 17u, empty);
}

[host]
void main()
{
    let output = RWStructuredBuffer<uint>::alloc(4);
    enqueue(RetainedEmpty(), output);

    // This empty type never crosses a host signature. Legalization removes its arguments,
    // but the remaining scalar and buffer arguments must retain their shader parameter indices.
    dispatch(
        writeValues<ErasedEmpty>,
        uint3(2, 1, 1))(ErasedEmpty(), 2u, ErasedEmpty(), output, 41u, ErasedEmpty());

    for (uint i = 0; i < 4; ++i)
        print(output.readBack(i));
}

// CHECK: 17
// CHECK: 18
// CHECK: 41
// CHECK: 42
