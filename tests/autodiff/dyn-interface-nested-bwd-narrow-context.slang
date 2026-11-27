// Like dyn-interface-nested-bwd.slang, but `Path` only ever holds a `Diffuse`, so its context must
// be widened to the payload type, and `Glossy` checkpoints its `IBsdf` in an array. fxc (dx11)
// cannot unroll a nested dispatch in a loop.

//TEST:COMPARE_COMPUTE(targets=all-dx11, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
dyn interface IBsdf
{
    [Differentiable]
    float eval(float x);
};

struct Diffuse : IBsdf
{
    float k;

    [Differentiable, NoDiffThis]
    float eval(float x) { return k * x; }
};

struct Plastic : IBsdf
{
    float k;

    [Differentiable, NoDiffThis]
    float eval(float x) { return k * x * x; }
};

// Reads the buffer so that the backward pass stores the result instead of recomputing it.
IBsdf makeDiffuse(uint id)
{
    return Diffuse(2.0f + float(inputBuffer[id]));
}

IBsdf makeBsdf(uint id)
{
    if (inputBuffer[id] == 0)
        return Diffuse(2.0f);
    return Plastic(3.0f);
}

[anyValueSize(16)]
dyn interface IIntegrator
{
    [Differentiable]
    float sample(float x);
};

struct Path : IIntegrator
{
    uint bsdfId;

    [Differentiable, NoDiffThis]
    float sample(float x)
    {
        IBsdf bsdf = no_diff makeDiffuse(bsdfId);
        return bsdf.eval(x);
    }
};

struct Glossy : IIntegrator
{
    uint bsdfId;

    [Differentiable, NoDiffThis]
    float sample(float x)
    {
        float result = 0.0f;
        for (uint i = 0; i < 3; i++)
        {
            IBsdf bsdf = no_diff makeBsdf(bsdfId);
            result += bsdf.eval(x);
        }
        return result;
    }
};

IIntegrator makeIntegrator(uint id)
{
    if (inputBuffer[id] == 0)
        return Path(id);
    return Glossy(id);
}

[Differentiable]
float render(IIntegrator integrator, float x)
{
    return integrator.sample(x);
}

[shader("compute"), numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    IIntegrator integrator = makeIntegrator(tid.x);
    var dp = diffPair(1.0f, 0.0f);
    bwd_diff(render)(integrator, dp, 1.0f);
    outputBuffer[tid.x] = dp.d;
}

// Path with Diffuse 2x -> 2. Glossy with three Plastic 3x^2 -> 3 * 6x = 18.
// CHECK: 2.0
// CHECK-NEXT: 18.0
