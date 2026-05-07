// extension-to-interface.slang

// Test that an extension can be applied to all types conforming to an interface
// using the `extension<T:IFoo> T { ... }` syntax.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IHasValue
{
    int getValue();
}

extension<T : IHasValue> T
{
    int getValuePlusTen()
    {
        return this.getValue() + 10;
    }
}

struct TypeA : IHasValue
{
    int a;
    int getValue() { return a; }
}

struct TypeB : IHasValue
{
    int b;
    int getValue() { return b * 2; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    TypeA ta = { 5 };
    TypeB tb = { 3 };

    // CHECK: 15
    outputBuffer[0] = ta.getValuePlusTen();

    // CHECK: 16
    outputBuffer[1] = tb.getValuePlusTen();
}
