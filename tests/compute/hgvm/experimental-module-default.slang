//TEST:INTERPRET(hgvm): -api vulkan -O0

// HGVM is itself experimental, so its compiler and shader-JIT sessions enable experimental Slang
// modules without requiring an additional slangi command-line option. The test also exercises the
// persistent HGVM tool's downstream shader optimization setting.
import slang.neural;

[shader("compute")]
[numthreads(1, 1, 1)]
void writeValue(RWStructuredBuffer<float> output)
{
    InlineVector<float, 1> value = InlineVector<float, 1>(2.0);
    output[0] = value[0];
}

[host]
void main()
{
    RWStructuredBuffer<float> output = RWStructuredBuffer<float>::alloc(1);
    dispatch(writeValue, uint3(1, 1, 1))(output);
    if (output.readBack(0) != 2.0)
    {
        String message = "experimental module was not available to HGVM";
        hgbc_asm
        {
            OpFail $message;
        };
    }
}
