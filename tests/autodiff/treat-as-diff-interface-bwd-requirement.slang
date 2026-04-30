//TEST:COMPILE: -target hlsl -profile cs_6_0 -entry computeMain -no-codegen

// A [TreatAsDifferentiable] method can satisfy an interface requirement that
// asks for a backward-differentiable method. Synthesis may discover the same
// requirement witness through more than one path; this should not assert.

interface IModule<T : IDifferentiable>
{
    [BackwardDifferentiable]
    T forward(T input);
}

struct InferenceModule : IModule<float>
{
    [TreatAsDifferentiable]
    float forward(float input)
    {
        return input;
    }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    InferenceModule module;
    outputBuffer[dispatchThreadID.x] = module.forward(1.0);
}
