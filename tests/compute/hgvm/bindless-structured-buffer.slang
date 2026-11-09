//TEST:INTERPRET(hgvm): -api vulkan
//TEST:INTERPRET(hgvm): -api d3d12
//TEST:INTERPRET(hgvm): -api cuda
//TEST:INTERPRET(hgvm): -api cpu

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

struct BindlessArguments
{
    uint addend;
    Array<StructuredBuffer<uint>.Handle, 2> inputs;
    RWStructuredBuffer<uint>.Handle firstOutput;
    RWStructuredBuffer<uint>.Handle secondOutput;
    StructuredBuffer<uint>.Handle empty;
    bool useEmpty;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeThroughHandles(uniform BindlessArguments arguments)
{
    uint elementCount;
    uint elementStride;
    arguments.inputs[0].GetDimensions(elementCount, elementStride);
    uint value = arguments.inputs[0][0] + arguments.inputs[1][0] + arguments.addend +
                 elementCount + elementStride;
    if (arguments.useEmpty)
    {
        arguments.empty.GetDimensions(elementCount, elementStride);
        value += elementCount;
    }
    arguments.firstOutput[0] = value + 1;
    arguments.secondOutput[0] = value + 2;
}

StructuredBuffer<uint>.Handle makeInputHandle(uint value)
{
    RuntimeArray<uint> source = RuntimeArray<uint>::alloc(1);
    source[0] = value;
    StructuredBuffer<uint> input = StructuredBuffer<uint>::alloc(1);
    copy(input, 0, source, 0, 1);
    return input.getDescriptorHandle();
}

[host]
void main()
{
    RWStructuredBuffer<uint> firstOutput = RWStructuredBuffer<uint>::alloc(1);
    RWStructuredBuffer<uint> secondOutput = RWStructuredBuffer<uint>::alloc(1);

    BindlessArguments arguments;
    arguments.addend = 3;
    arguments.inputs[0] = makeInputHandle(40);
    arguments.inputs[1] = makeInputHandle(2);
    arguments.firstOutput = firstOutput.getDescriptorHandle();
    arguments.secondOutput = secondOutput.getDescriptorHandle();
    arguments.useEmpty = false;
    dispatch(writeThroughHandles, uint3(1, 1, 1))(arguments);

    RWStructuredBuffer<uint> recoveredFirstOutput = *arguments.firstOutput;

    check(
        recoveredFirstOutput.readBack(0) == 51,
        "HGVM recovered the wrong resource from a read-write descriptor handle");
    check(
        secondOutput.readBack(0) == 52,
        "HGVM aliased two distinct descriptor handles of the same type");
}
