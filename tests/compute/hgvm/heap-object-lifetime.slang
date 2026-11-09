//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct BufferHolder
{
    RWStructuredBuffer<uint> buffer;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValue(RWStructuredBuffer<uint> output, uint value)
{
    output[0] = value;
}

[noinline]
RWStructuredBuffer<uint> passThrough(RWStructuredBuffer<uint> value)
{
    return value;
}

[noinline]
BufferHolder makeHolder(RWStructuredBuffer<uint> value)
{
    BufferHolder result;
    result.buffer = value;
    return result;
}

[host]
void main()
{
    RWStructuredBuffer<uint> original = RWStructuredBuffer<uint>::alloc(1);
    RWStructuredBuffer<uint> alias = passThrough(original);
    BufferHolder holder = makeHolder(alias);

    dispatch(writeValue, uint3(1, 1, 1))(holder.buffer, 17);
    print(readBack(original, 0));

    holder.buffer = RWStructuredBuffer<uint>::alloc(1);
    dispatch(writeValue, uint3(1, 1, 1))(holder.buffer, 23);
    print(readBack(holder.buffer, 0));
    print(readBack(original, 0));

    Array<RWStructuredBuffer<uint>, 4> buffers =
        __makeArray(original, holder.buffer, original, holder.buffer);
    dispatch(writeValue, uint3(1, 1, 1))(buffers[0], 31);
    print(readBack(original, 0));

    uint count;
    uint stride;
    holder.buffer.GetDimensions(count, stride);
    print(count);
    print(stride);
}

// CHECK: 17
// CHECK-NEXT: 23
// CHECK-NEXT: 17
// CHECK-NEXT: 31
// CHECK-NEXT: 1
// CHECK-NEXT: 4
