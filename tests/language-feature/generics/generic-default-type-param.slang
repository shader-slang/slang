// Test generic type parameters with default type values.
// When no explicit type argument is provided, the default type should be used.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

struct Holder<T = int>
{
    T value;

    T getValue() { return value; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Use default type (int) by omitting type argument
    Holder<> defaultHolder;
    defaultHolder.value = 42;
    outputBuffer[0] = defaultHolder.getValue();
    // CHECK: 42

    // Explicit type argument overrides the default
    Holder<int> explicitHolder;
    explicitHolder.value = 99;
    outputBuffer[1] = explicitHolder.getValue();
    // CHECK: 99
}
