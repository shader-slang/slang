// struct-inheritance.slang

// Test struct inheriting from a single base struct plus interface conformance.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

#pragma warning(disable:30816)

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IScalable
{
    int scaled(int factor);
}

struct Base
{
    int value;

    int getValue()
    {
        return value;
    }
}

// Derived inherits from Base struct and conforms to IScalable interface
struct Derived : Base, IScalable
{
    int extra;

    int scaled(int factor)
    {
        return value * factor;
    }

    int sum()
    {
        return value + extra;
    }
}

int applyScale<T : IScalable>(T obj, int factor)
{
    return obj.scaled(factor);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Derived d;
    d.value = 10;
    d.extra = 5;

    // Access inherited field via inherited method
    // CHECK: 10
    outputBuffer[0] = d.getValue();

    // Access derived method
    // CHECK: 15
    outputBuffer[1] = d.sum();

    // Interface method
    // CHECK: 30
    outputBuffer[2] = d.scaled(3);

    // Use through generic interface constraint
    // CHECK: 50
    outputBuffer[3] = applyScale(d, 5);
}
