//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm
    {
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
void copyValues(
    uniform Array<uint, 4> input,
    RWStructuredBuffer<Array<uint, 4>> output,
    uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (dispatchThreadId.x == 0)
        output[0] = input;
}

[host]
void main()
{
    Array<uint, 4> input = __makeArray(5u, 10u, 15u, 20u);
    RWStructuredBuffer<Array<uint, 4>> output =
        RWStructuredBuffer<Array<uint, 4>>::alloc(1);
    dispatch(copyValues, uint3(1, 1, 1))(input, output);

    Array<uint, 4> result = readBack(output, 0);
    check(result[0] == 5 && result[1] == 10 && result[2] == 15 && result[3] == 20,
          "fixed array dispatch or readback failed");
}
