//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm {
        OpFail $message;
    };
}

void check(bool condition, String message)
{
    if (!condition)
        fail(message);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValue(
    RWStructuredBuffer<uint> output,
    uint value,
    uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (dispatchThreadId.x == 0)
        output[0] = value;
}

[host]
void main()
{
    RWStructuredBuffer<uint> output = RWStructuredBuffer<uint>::alloc(1);
    dispatch(writeValue, uint3(1, 1, 1))(output, 42);
    check(readBack(output, 0) == 42, "HGVM compute readback returned the wrong value");
}
