// createDynamicObject does not currently support generic interface types.
// Attempting to use createDynamicObject<ICalculator<float>> fails because
// the generic specialization cannot be resolved for the dynamic object
// construction path.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

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

ICalculator<float> createCalc(uint id)
{
    // CHECK: ([[# @LINE+1]]): error 39999
    return createDynamicObject<ICalculator<float>>(id);
}

float test(uint id, float a, float b)
{
    let calc = createCalc(id);
    return calc.compute(a, b);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = test(0, 3.0, 4.0);
    outputBuffer[1] = test(1, 3.0, 4.0);
}
