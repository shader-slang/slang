// An Optional<interface> whose type-flow set contains one concrete type plus `none` should keep
// the ordinary tagged-union behavior when its payload is stored as the concrete type. Exercise
// both tags through full compilation and execution.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Keeping a resource handle in the implementation makes this a regression test for the physical
// representation too: on non-bindless targets a resource cannot be bit-cast through AnyValue,
// but it can be stored and used normally when the optional's single payload is lowered directly
// to FooImpl.
//TEST_INPUT:ubuffer(data=[42], stride=4):name=inputBuffer
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
StructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    int getValue();
}

struct FooImpl : IFoo
{
    StructuredBuffer<int> values;
    int getValue() { return values[0]; }
}

Optional<IFoo> makeValue(bool hasValue)
{
    if (hasValue)
    {
        FooImpl result = {inputBuffer};
        return {result};
    }
    return none;
}

struct Holder
{
    Optional<IFoo> value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Holder holder;
    holder.value = makeValue(true);

    outputBuffer[0] = holder.value.hasValue ? holder.value.value.getValue() : -1;
    // CHECK: 42

    holder.value = makeValue(false);

    outputBuffer[1] = holder.value.hasValue ? holder.value.value.getValue() : -1;
    // CHECK-NEXT: -1
}
