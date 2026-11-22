//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK): -output-using-type

// Test that calls to overloaded generic functions can be resolved to prefer the
// generic candidate with more specialized constraints.
interface IBase
{
    float get();
}
interface IDerived : IBase
{

}
float process<T>(T v)
{
    return 0.0;
}
float process<T : IBase>(T v)
{
    return v.get();
}

float process<T : IDerived>(T v)
{
    return v.get() + 1.0;
}

struct D : IDerived
{
    float get() { return 1.0; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    D d;
    outputBuffer[0] = process(d);
    // CHECK: 2.0
}
