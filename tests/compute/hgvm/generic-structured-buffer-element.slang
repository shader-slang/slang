//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

struct Payload
{
    uint value;
}

struct Wrapper<T>
{
    T payload;
    uint4 bounds;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeWrapped(
    RWStructuredBuffer<Wrapper<Payload>> buffer,
    uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint index = dispatchThreadId.x;
    Wrapper<Payload> item;
    item.payload.value = index + 1;
    item.bounds = uint4(10 + index, 20 + index, 30 + index, 40 + index);
    buffer[index] = item;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void readWrapped(
    RWStructuredBuffer<Wrapper<Payload>> buffer,
    RWStructuredBuffer<uint> output,
    uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint index = dispatchThreadId.x;
    Wrapper<Payload> item = buffer[index];
    output[index * 2] = item.payload.value;
    output[index * 2 + 1] = item.bounds.x;
}

[host]
void main()
{
    const uint elementCount = 4;
    RWStructuredBuffer<Wrapper<Payload>> buffer =
        RWStructuredBuffer<Wrapper<Payload>>::alloc(elementCount);
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(elementCount * 2);

    dispatch(writeWrapped, uint3(elementCount, 1, 1))(buffer);
    dispatch(readWrapped, uint3(elementCount, 1, 1))(buffer, output);

    for (uint i = 0; i < elementCount; ++i)
    {
        if (readBack(output, i * 2) != i + 1 || readBack(output, i * 2 + 1) != i + 10)
            fail("specialized generic structured-buffer elements used the wrong layout");
    }
}
