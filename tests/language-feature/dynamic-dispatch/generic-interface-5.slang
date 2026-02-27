// Generic interfaces specialized with int should support dynamic dispatch.
// Tests ICalculator<T> with both float and int specializations to verify
// that different type arguments work correctly through the same interface.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface ICalculator<T>
{
    T compute(T a, T b);
}

struct IntAdder : ICalculator<int>
{
    int compute(int a, int b) { return a + b; }
}

struct IntMultiplier : ICalculator<int>
{
    int compute(int a, int b) { return a * b; }
}

struct IntSubtractor : ICalculator<int>
{
    int compute(int a, int b) { return a - b; }
}

ICalculator<int> makeCalc(int id)
{
    if (id == 0) return IntAdder();
    else if (id == 1) return IntMultiplier();
    else return IntSubtractor();
}

int test(int id, int a, int b)
{
    let calc = makeCalc(id);
    return calc.compute(a, b);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 3 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = test(0, 10, 3); // CHECK: 13
    outputBuffer[1] = test(1, 10, 3); // CHECK: 30
    outputBuffer[2] = test(2, 10, 3); // CHECK: 7
}
