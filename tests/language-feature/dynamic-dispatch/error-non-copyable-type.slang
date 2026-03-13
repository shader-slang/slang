//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

// Test that types with non-copyable members cannot be used in dynamic dispatch

interface IProcessor
{
    float process(float x);
}

[__NonCopyableType]
struct NonCopyableData
{
    float value;
};

// CHECK: error 41014
struct ProcessorWithNonCopyable : IProcessor
{
    NonCopyableData data;
    
    float process(float x)
    {
        return data.value * x;
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
        ProcessorWithNonCopyable pwn;
        return pwn;
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IProcessor proc = createProcessor(0);
    outputBuffer[0] = proc.process(2.0);
}
