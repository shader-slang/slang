// Generic interfaces with concrete type specialization should support
// dynamic dispatch. This tests ICalculator<T> specialized as
// ICalculator<float> with a factory function returning the interface type.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface ICalculator<T>
{
    T compute(T a, T b);
}

struct Adder : ICalculator<float>
{
    float compute(float a, float b) { return a + b; }
}

struct Multiplier : ICalculator<float>
{
    float compute(float a, float b) { return a * b; }
}

ICalculator<float> makeCalc(int id)
{
    if (id == 0) return Adder();
    else return Multiplier();
}

float test(int id, float a, float b)
{
    let calc = makeCalc(id);
    return calc.compute(a, b);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = test(0, 3.0, 4.0); // CHECK: 7.0
    outputBuffer[1] = test(1, 3.0, 4.0); // CHECK: 12.0
}
