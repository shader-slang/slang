//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

// Test that types with opaque members (like Texture2D) cannot be used in dynamic dispatch

interface IProcessor
{
    float process(float x);
}

// CHECK: error 41014
struct ProcessorWithTexture : IProcessor
{
    Texture2D<float4> myTexture;
    
    float process(float x)
    {
        return myTexture.Load(int3(0, 0, 0)).r * x;
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
        ProcessorWithTexture pwt;
        // Note: Can't actually construct with texture, but compiler should error before that
        return pwt;
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IProcessor proc = createProcessor(0);
    outputBuffer[0] = proc.process(2.0);
}
