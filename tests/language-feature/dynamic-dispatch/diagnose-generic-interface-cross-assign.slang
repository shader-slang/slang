// Different specializations of a generic interface are distinct types.
// Assigning IProcessor<int> to IProcessor<float> is correctly rejected.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

interface IProcessor<T : IArithmetic>
{
    T process(T input);
}

struct FloatDoubler : IProcessor<float>
{
    float process(float input) { return input * 2.0; }
}

struct IntDoubler : IProcessor<int>
{
    int process(int input) { return input * 2; }
}

RWStructuredBuffer<float> outputBuffer;

// CHECK: ([[# @LINE+5]]): error 30019: expected an expression of type 'IProcessor<float>', got 'IntDoubler'
[numthreads(1, 1, 1)]
void computeMain()
{
    IProcessor<float> obj = FloatDoubler();
    obj = IntDoubler();
    outputBuffer[0] = obj.process(5.0);
}
