//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

// Verify that a generic typealias with where-clause constraints
// correctly enforces the interface requirement and computes results.

interface IValueProvider
{
    int getValue();
}

struct ConstantProvider : IValueProvider
{
    int stored;

    int getValue() { return stored; }
}

struct DoubleProvider : IValueProvider
{
    int base;

    int getValue() { return base * 2; }
}

struct Wrapper<T : IValueProvider>
{
    T provider;

    int extract() { return provider.getValue(); }
}

typealias WrappedProvider<T : IValueProvider> = Wrapper<T>;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ConstantProvider cp;
    cp.stored = 42;
    WrappedProvider<ConstantProvider> w1;
    w1.provider = cp;
    // CHECK: 42
    outputBuffer[0] = w1.extract();

    DoubleProvider dp;
    dp.base = 10;
    WrappedProvider<DoubleProvider> w2;
    w2.provider = dp;
    // CHECK: 20
    outputBuffer[1] = w2.extract();
}
