// Test that a where clause constraining an associated type to a concrete type
// enables operations that depend on knowing the concrete type (e.g. arithmetic).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

interface IProvider
{
    associatedtype DataType;
    DataType getData();
}

// The equality constraint `T.DataType == int` lets the body use int arithmetic
// on the value returned by getData(), which would not type-check without it.
int transform<T : IProvider>(T provider, int multiplier)
    where T.DataType == int
{
    return provider.getData() * multiplier + 1;
}

struct IntProvider : IProvider
{
    typealias DataType = int;
    int stored;
    int getData() { return stored; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IntProvider p;
    p.stored = 5;

    outputBuffer[0] = transform(p, 10);
    // CHECK: 51
}
