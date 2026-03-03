// A generic interface requires all type arguments to be specified.
// Using a generic interface without type arguments is an error.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

interface IProcessor<T : IArithmetic>
{
    T process(T input);
}

struct Doubler : IProcessor<float>
{
    float process(float input) { return input * 2.0; }
}

RWStructuredBuffer<float> outputBuffer;

// CHECK: ([[# @LINE+4]]): error 30400
[numthreads(1, 1, 1)]
void computeMain()
{
    IProcessor obj = Doubler();
    outputBuffer[0] = obj.process(5.0);
}
