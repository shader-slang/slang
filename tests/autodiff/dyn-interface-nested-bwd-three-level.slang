// Like dyn-interface-nested-bwd.slang, but three levels deep, so the inner contexts cross
// dispatches that an earlier type-flow run has already rewritten.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

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

[anyValueSize(16)]
dyn interface ITex
{
    [Differentiable]
    float lookup(float x);
};
struct TexA : ITex { [Differentiable, NoDiffThis] float lookup(float x) { return 2.0f * x; } };
struct TexB : ITex { [Differentiable, NoDiffThis] float lookup(float x) { return x * x; } };
ITex makeTex(uint id) { if (inputBuffer[id] == 0) return TexA(); return TexB(); }

[anyValueSize(16)]
dyn interface IMat
{
    [Differentiable]
    float eval(float x);
};
struct Textured : IMat
{
    uint texId;
    [Differentiable, NoDiffThis]
    float eval(float x) { ITex t = no_diff makeTex(texId); return t.lookup(x) * x; }
};
struct Flat : IMat { [Differentiable, NoDiffThis] float eval(float x) { return x; } };
IMat makeMat(uint id) { if (inputBuffer[id] > 1) return Flat(); return Textured(id); }

struct Path : IIntegrator
{
    uint matId;
    [Differentiable, NoDiffThis]
    float sample(float x)
    {
        IMat m = no_diff makeMat(matId);
        return m.eval(x);
    }
};

struct Constant : IIntegrator
{
    [Differentiable, NoDiffThis]
    float sample(float x) { return x; }
};

IIntegrator makeIntegrator(uint id)
{
    if (inputBuffer[id] > 1)
        return Constant();
    return Path(id);
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

// TexA 2x * x -> 4x = 4, TexB x^2 * x -> 3x^2 = 3.
// CHECK: 4.0
// CHECK-NEXT: 3.0
