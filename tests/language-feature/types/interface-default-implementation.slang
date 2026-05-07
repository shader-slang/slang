// interface-default-implementation.slang

// Test default method implementation in an interface, and overriding the default.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IGreeter
{
    int getBase();

    // Default implementation that uses getBase().
    int getDoubled()
    {
        return getBase() * 2;
    }
}

struct UsesDefault : IGreeter
{
    int val;
    int getBase() { return val; }
    // Uses the default getDoubled().
}

struct OverridesDefault : IGreeter
{
    int val;
    int getBase() { return val; }

    // Override the default implementation.
    override int getDoubled()
    {
        return getBase() * 3;
    }
}

int callDoubled<T : IGreeter>(T obj)
{
    return obj.getDoubled();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    UsesDefault a;
    a.val = 5;
    outputBuffer[0] = callDoubled(a);
    // CHECK: 10

    OverridesDefault b;
    b.val = 5;
    outputBuffer[1] = callDoubled(b);
    // CHECK: 15
}
