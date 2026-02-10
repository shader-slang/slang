//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

// Test that types with buffer members cannot be used in dynamic dispatch

interface IProcessor
{
    float process(float x);
}

// CHECK: error 41014
struct ProcessorWithBuffer : IProcessor
{
    StructuredBuffer<float> myBuffer;
    
    float process(float x)
    {
        return myBuffer[0] * x;
    }
};

struct ValidProcessor : IProcessor
{
    float factor;
    
    float process(float x)
    {
        return x * factor;
    }
};

IProcessor createProcessor(uint id)
{
    if (id == 0)
    {
        ValidProcessor vp;
        vp.factor = 2.0;
        return vp;
    }
    else
    {
        ProcessorWithBuffer pwb;
        return pwb;
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IProcessor proc = createProcessor(0);
    outputBuffer[0] = proc.process(2.0);
}
