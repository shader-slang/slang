// extension-generic-type.slang

// Test that a generic extension can be applied to a generic type,
// using interface constraints on type parameters.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IValue
{
    int getValue();
}

struct SimpleVal : IValue
{
    int v;
    int getValue() { return v; }
}

struct Wrapper<T>
{
    T inner;
}

extension<T : IValue> Wrapper<T>
{
    int getInnerValue()
    {
        return inner.getValue();
    }

    int getDoubled()
    {
        return inner.getValue() * 2;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    SimpleVal sv = { 7 };
    Wrapper<SimpleVal> w = { sv };

    // CHECK: 7
    outputBuffer[0] = w.getInnerValue();

    // CHECK: 14
    outputBuffer[1] = w.getDoubled();
}
