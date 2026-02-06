//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

// Test that types with unsized array members cannot be used in dynamic dispatch
// Note: The error is caught earlier at variable instantiation, not during packing

interface IProcessor
{
    float process(float x);
}

struct ProcessorWithUnsized : IProcessor
{
    float values[];  // Unsized array
    
    float process(float x)
    {
        return values[0] * x;
    }
};

struct ValidProcessor : IProcessor
{
    float factor;
    float fixedArray[10];  // Sized arrays are OK
    
    float process(float x)
    {
        return x * factor * fixedArray[0];
    }
};

IProcessor createProcessor(uint id)
{
    if (id == 0)
    {
        ValidProcessor vp;
        vp.factor = 2.0;
        vp.fixedArray[0] = 1.0;
        return vp;
    }
    else
    {
        // CHECK: error 30071
        ProcessorWithUnsized pwu;  // Error: cannot instantiate variable of unsized type
        return pwu;
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IProcessor proc = createProcessor(0);
    outputBuffer[0] = proc.process(2.0);
}
