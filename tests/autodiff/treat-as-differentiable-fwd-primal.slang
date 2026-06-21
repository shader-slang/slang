// Regression test: forward-differentiating a [TreatAsDifferentiable] function must
// preserve the *primal* value f(x). The synthesized trivial forward-derivative used
// to default-construct (zero) the primal, which collapsed both the primal and any
// surrounding derivative to zero whenever a value flowed through such a function.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Stand-in for a texture lookup: differentiable for type-checking, but its own
// derivative is deliberately ignored.
[TreatAsDifferentiable]
float tex(float u)
{
    return 2.0 * u + 1.0;
}

// tex() is treated as a constant w.r.t. autodiff, but the surrounding `/ x` is a
// genuine differentiable op whose derivative must survive.
[Differentiable]
float divAfterTex(float x)
{
    float s = tex(x);   // primal must be tex(x), differential is 0
    return s / x;
}

// Same, but through a dynamic interface dispatch (mirrors the real-world case where
// the call goes through a `dyn interface` witness).
interface IFoo
{
    [Differentiable]
    float f(float v);
}

struct Tex : IFoo
{
    [TreatAsDifferentiable]
    float f(float v) { return 2.0 * v + 1.0; }
}

[Differentiable]
float divAfterTexIface(IFoo o, float x)
{
    float s = o.f(x);
    return s / x;
}

// A trivial derivative must also force `inout` tangents to zero: the call's
// contribution is df/dx == 0, so the modified value's differential becomes zero
// rather than threading the incoming tangent through unchanged.
[TreatAsDifferentiable]
void scaleInout(inout float x)
{
    x = 3.0 * x;
}

[Differentiable]
float useInout(float x)
{
    float v = x;       // v = (2, 1)
    scaleInout(v);     // primal -> 6; tangent must be forced to 0
    return v;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // f(2) = 5, so divAfterTex(2) = 5/2 = 2.5
    // d/dx (tex/x) with tex treated constant = -tex/x^2 = -5/4 = -1.25
    var dp = __fwd_diff(divAfterTex)(diffPair(2.0, 1.0));

    Tex t;
    var dpIface = __fwd_diff(divAfterTexIface)(t, diffPair(2.0, 1.0));

    // primal preserved (6), inout tangent forced to zero
    var dpInout = __fwd_diff(useInout)(diffPair(2.0, 1.0));

    outputBuffer[0] = dp.p;       // CHECK: 2.5
    outputBuffer[1] = dp.d;       // CHECK: -1.25
    outputBuffer[2] = dpIface.p;  // CHECK: 2.5
    outputBuffer[3] = dpIface.d;  // CHECK: -1.25
    outputBuffer[4] = dpInout.p;  // CHECK: 6.0
    outputBuffer[5] = dpInout.d;  // CHECK: 0.0
}
