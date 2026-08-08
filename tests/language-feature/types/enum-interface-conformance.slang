// enum-interface-conformance.slang

// Test an enum conforming to an interface via extension.
// Demonstrates dispatching interface methods on enum values.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IPriority
{
    int priority();
}

enum Severity
{
    Low,
    Medium,
    High,
    Critical
}

extension Severity : IPriority
{
    int priority()
    {
        switch (this)
        {
        case Severity.Low:      return 1;
        case Severity.Medium:   return 5;
        case Severity.High:     return 8;
        case Severity.Critical: return 10;
        default:                return 0;
        }
    }
}

int getPriority<T : IPriority>(T item)
{
    return item.priority();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1
    outputBuffer[0] = getPriority(Severity.Low);
    // CHECK: 5
    outputBuffer[1] = getPriority(Severity.Medium);
    // CHECK: 8
    outputBuffer[2] = getPriority(Severity.High);
    // CHECK: 10
    outputBuffer[3] = getPriority(Severity.Critical);
}
