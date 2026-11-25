//TEST:SIMPLE_EX: -lang hlsl tests/language-feature/hlsl-default-this-immutable-receiver.slang -target spirv -stage compute -entry computeMain -o hlsl-default-this-immutable-receiver.spv

// Unannotated HLSL methods are nonmutating, so we can call them on immutable receivers.
// We exercise constant-buffer members, read-only buffer elements, const locals, and temporaries.

struct Helper
{
    int value;

    int read()
    {
        return value;
    }

    int readThroughThis()
    {
        return this.read();
    }

    int readImplicitly()
    {
        return read();
    }
};

cbuffer C
{
    Helper helper;
};

StructuredBuffer<Helper> input;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    const Helper local = {1};
    output[0] = helper.read() + input[0].read() + local.read() + Helper(2).read()
        + helper.readThroughThis() + helper.readImplicitly();
}
