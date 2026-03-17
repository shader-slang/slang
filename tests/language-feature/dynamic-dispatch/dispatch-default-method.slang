// Default interface method implementations through dynamic dispatch.
// Verifies that a type using the default keeps the default behavior,
// while a type that overrides it gets the overridden behavior.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IGreeter
{
    int greet(int x);
    int defaultGreet(int x) { return x + 100; }
}

struct DefaultImpl : IGreeter
{
    int greet(int x) { return x + 1; }
}

struct OverrideImpl : IGreeter
{
    int greet(int x) { return x + 2; }
    override int defaultGreet(int x) { return x + 200; }
}

IGreeter makeGreeter(int id)
{
    if (id == 0)
        return DefaultImpl();
    return OverrideImpl();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IGreeter def = makeGreeter(0);
    IGreeter ovr = makeGreeter(1);

    outputBuffer[0] = def.greet(10);
    // CHECK: 11
    outputBuffer[1] = def.defaultGreet(10);
    // CHECK: 110
    outputBuffer[2] = ovr.greet(10);
    // CHECK: 12
    outputBuffer[3] = ovr.defaultGreet(10);
    // CHECK: 210
}
